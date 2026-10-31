#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 yceachan
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise the production QML drag components through the X11 native DnD path."""
import ctypes
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

x11 = ctypes.CDLL("libX11.so.6")
xtst = ctypes.CDLL("libXtst.so.6")
x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
x11.XOpenDisplay.restype = ctypes.c_void_p
x11.XFlush.argtypes = [ctypes.c_void_p]
x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
xtst.XTestFakeMotionEvent.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_ulong]
xtst.XTestFakeButtonEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]
xtst.XTestFakeKeyEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]
display = x11.XOpenDisplay(None)
if not display:
    raise RuntimeError("Run this test under xvfb-run")

def move(x, y):
    xtst.XTestFakeMotionEvent(display, -1, x, y, 0)
    x11.XFlush(display)
    time.sleep(0.15)

def button(down, number=1):
    xtst.XTestFakeButtonEvent(display, number, int(down), 0)
    x11.XFlush(display)
    time.sleep(0.15)

def drag(source, target, number=1):
    move(*source)
    button(True, number)
    move(source[0] + 24, source[1] + 3)
    time.sleep(0.25)  # grabToImage completes before crossing surfaces
    move(*target)
    time.sleep(0.25)
    button(False, number)
    time.sleep(0.3)

env = dict(os.environ, QT_QPA_PLATFORM="xcb", QT_FORCE_STDERR_LOGGING="1", QT_QUICK_CONTROLS_STYLE="org.kde.desktop")
qml = sys.argv[1]
scene = str(Path(__file__).with_name("drag.qml"))
with tempfile.TemporaryFile(mode="w+") as output:
    process = subprocess.Popen([qml, scene], stdout=output, stderr=output, env=env)
    try:
        time.sleep(1.5)
        assert process.poll() is None, "QML harness failed to start"
        # Verify plain hover on the production entrance before starting drags.
        move(440, 250)
        time.sleep(0.7)
        move(10, 10)
        # Single click and sub-threshold movement preserve activation.
        move(100, 250)
        button(True)
        move(102, 251)
        button(False)
        # System widgets and non-left buttons cannot initiate a native drag.
        drag((200, 250), (440, 250))
        drag((100, 250), (440, 250), 3)
        # The production application entrance opens its own popup.
        move(440, 250)
        time.sleep(0.5)
        output.seek(0)
        startup = output.read()
        bounds_lines = [line.split("OVERFLOW_BOUNDS ", 1)[1] for line in startup.splitlines() if "OVERFLOW_BOUNDS " in line]
        assert bounds_lines, "Independent application popup did not open:\n" + startup
        bounds = json.loads(bounds_lines[-1])

        # Collapse on the real entrance, then cross its popup boundary both ways.
        drag((100, 250), (440, 250))
        drag((100, 250), (bounds["blankX"], bounds["blankY"]))
        drag((bounds["appX"], bounds["appY"]), (320, 290))
        # Application-only insertion on both halves of a target icon.
        drag((100, 250), (280, 250))
        drag((100, 250), (320, 250))
        # Escape cancels an active native drag without committing.
        move(100, 250)
        button(True)
        move(130, 253)
        time.sleep(0.3)
        xtst.XTestFakeKeyEvent(display, 9, 1, 0)  # Escape on the Xvfb default keymap
        xtst.XTestFakeKeyEvent(display, 9, 0, 0)
        x11.XFlush(display)
        button(False)
        time.sleep(0.4)
        output.seek(0)
        log = output.read()
        print(log)
        commits = [line for line in log.splitlines() if "COMMIT " in line]
        assert len(commits) == 5, f"Expected 5 successful drops, got {commits}"
        assert "COMMIT panel-app true" in commits[0], commits
        assert "COMMIT panel-app true" in commits[1], commits
        assert "COMMIT folded-app false" in commits[2], commits
        assert "COMMIT panel-app false target-app false" in commits[3], commits
        assert "COMMIT panel-app false target-app true" in commits[4], commits
        assert log.count("CLICK 1") == 1, "Native drag must not activate the application"
        assert "OVERFLOW_OPEN" in log, "The independent application popup must be visible"
        assert "ReferenceError" not in log and "TypeError" not in log, log
    finally:
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        x11.XCloseDisplay(display)
