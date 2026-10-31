# SPDX-FileCopyrightText: 2026 yceachan
# SPDX-License-Identifier: GPL-2.0-or-later
# Modified 2026-10-31: standalone tray build and test integration.

ecm_add_tests(systemtraymodeltest.cpp applicationtraytest.cpp
    LINK_LIBRARIES systemtray_static
    Qt::Test
)
