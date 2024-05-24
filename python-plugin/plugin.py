import sys
import os

venv_path = os.path.dirname(os.path.realpath(__file__))
venv_site_packages = os.path.join(venv_path, "lib", "python3.12", "site-packages")

sys.path.insert(0, venv_site_packages)

import gdb
import socket
import json

class QEMUMonitorProtocol:
    def __init__(self, address):
        self.address = address
        self.sock = None
        self.connect()

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(self.address)
        self._recv()

    def _recv(self):
        data = b""
        while True:
            chunk = self.sock.recv(4096)
            data += chunk
            if b"\r\n" in chunk:
                break
        return json.loads(data.decode())

    def _send(self, cmd):
        self.sock.sendall(json.dumps(cmd).encode() + b"\r\n")

    def execute(self, cmd, args=None):
        command = {"execute": cmd}
        if args:
            command["arguments"] = args
        
        self._send(command)
        return self._recv()

class PLC_GetRegisters(gdb.Command):
    def __init__(self):
        super(PLC_GetRegisters, self).__init__("plc-regs", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        qmp = QEMUMonitorProtocol(("localhost", 4444))
        qmp.execute("qmp_capabilities")

        res = qmp.execute("human-monitor-command", {"command-line": "info registers", "cpu-index": 4})

        regs_all = res["return"]

        for reg in regs_all:
            gdb.write(reg)

breakpoints = {}
class PLC_SetBreakpoint(gdb.Command):
    def __init__(self):
        super(PLC_SetBreakpoint, self).__init__("plc-break", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        qmp = QEMUMonitorProtocol(("localhost", 4444))
        qmp.execute("qmp_capabilities")

        i = gdb.inferiors()[0]
        m = i.read_memory(int(arg, 16), 4)
        addr = m.tobytes()[::-1].hex()
        gdb.write(addr + "\n")
        breakpoints[arg[2:]] = m.tobytes()[::-1]
        
        i.write_memory(int(arg, 16), bytes.fromhex("feffffea"))
        gdb.write("Breakpoint set.\n")


class PLC_Continue(gdb.Command):
    def __init__(self):
        super(PLC_Continue, self).__init__("plc-cont", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        qmp = QEMUMonitorProtocol(("localhost", 4444))
        qmp.execute("qmp_capabilities")

        res = qmp.execute("human-monitor-command", {"command-line": "info registers", "cpu-index": 4})

        regs_all = res["return"]
        pc = regs_all.split(" ")[12].split("\n")[0].strip().split("=")[1]

        i = gdb.inferiors()[0]
        m = i.read_memory(int(pc, 16), 4)
        addr = m.tobytes()[::-1].hex()

        gdb.write(addr + "\n")
        if (addr == "eafffffe"):
            gdb.write(breakpoints[pc].hex() + "\n")
            i.write_memory(int(pc, 16), breakpoints[pc][::-1])

        qmp.execute("human-monitor-command", {"command-line": "cont"})


PLC_GetRegisters()
PLC_SetBreakpoint()
PLC_Continue()
gdb.execute("target remote localhost:1234")

