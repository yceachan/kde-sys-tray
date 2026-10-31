---
title: 折叠系统托盘
tags: [KDE, Plasma, QML]
note_types: document
created: 2026-10-30
updated: 2026-10-31

---

# 折叠系统托盘

基于 **Plasma 6.7.5 官方系统托盘**的独立派生组件，为应用图标增加 Windows 风格的拖拽收纳、移出固定与排序。应用使用独立箭头和紧凑图标浮层收纳。网络、音量、电池等 Plasma 内置组件及其“状态和通知”入口保留官方交互；两种弹窗分别管理。

组件名称为 **折叠系统托盘 / Windows-style System Tray**，插件标识为 `local.plasma.windowstray`。整体采用 **[GPL-2.0-or-later](LICENSE)** 开源；源码基线及各部分许可证见 [UPSTREAM.md](UPSTREAM.md)。

## 交互

| 操作 | 结果 |
| --- | --- |
| 悬停应用专用箭头约 300 ms | 展开仅含应用图标的五列浮层 |
| 左键按住应用图标并移动超过系统拖拽阈值 | 开始拖拽，抑制本次点击激活 |
| 拖到应用专用箭头或应用浮层空白处松手 | 将应用收纳到专用浮层 |
| 从应用浮层拖到托盘应用区松手 | 固定到应用区末尾；全部应用都已收纳时，拖拽会显示空白吸附位 |
| 拖到另一应用图标的前半或后半处松手 | 在对应位置插入，并采用目标所在区域的显隐状态 |
| 拖到有效目标之外松手，或按 Esc | 取消本次变更 |
| 单击、右键、中键、滚轮 | 沿用官方应用操作 |

默认横向布局依次为：**应用图标 → 带细边框的应用折叠箭头 → 系统组件 → 官方隐藏项目入口**。应用箭头在浮层为空时也保持可见，空浮层显示拖入提示。应用浮层不显示“状态和通知”标题或系统组件，图标名称通过提示显示。浮层图标与托盘应用图标共享尺寸，单元格随图标尺寸同步收缩或放大，并保留主题间距。

通过悬停打开的应用浮层，在鼠标离开箭头与浮层约 500 ms 后关闭，拖拽期间保持展开；点击可固定展开，再次点击、按 Esc 或点击外部可关闭。应用浮层与官方弹窗互斥显示；官方“保持打开”设置继续仅作用于官方弹窗。面板方向、反向排列与 RTL 按面板布局调整。

**可拖拽边界以官方模型类型为准：**

- `StatusNotifier`：允许拖拽，包括 KDE 应用及通过同一协议提供图标的服务。
- `Plasmoid`：网络、音量、电池、通知等内置组件，锁定拖拽与排序。
- `BackgroundApp`：沿用官方后台应用入口，本版锁定拖拽。

应用组成连续的可排序区；系统区内部保留通知入口优先及其余组件的官方相对顺序。应用启动后按保存的标识恢复位置；同一标识的多个实例共享偏好。未手动调整的应用继续采用官方的自动显隐策略。手动收纳的应用优先遵从收纳设置，包括启用“显示所有项目”的场景。

## 构建

目标环境为 Plasma / libplasma **6.7.5**、Qt **6.10+**、KDE Frameworks **6.30+**。构建配置精确检查 libplasma 版本；元数据中的小部件 API 声明沿用上游的 `6.0`。升级 Plasma 时应先对齐上游基线再重新构建。

Fedora 开发依赖：

```bash
sudo dnf install cmake ninja-build gcc-c++ extra-cmake-modules \
  libplasma-devel qt6-qtbase-devel qt6-qtbase-private-devel \
  qt6-qtdeclarative-devel qt6-qtwayland-devel \
  kf6-kconfig-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel \
  kf6-ki18n-devel kf6-kio-devel kf6-kjobwidgets-devel \
  kf6-kservice-devel kf6-kwindowsystem-devel kf6-kxmlgui-devel kf6-kpackage-devel \
  kf6-kstatusnotifieritem python3 xorg-x11-server-Xvfb libXtst
```

在仓库根目录执行：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
cmake --build build -j 4
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

产物为 `build/bin/plasma/applets/local.plasma.windowstray.so`。测试包含上游模型测试、应用显隐与持久化测试、Xvfb 下跨窗口的原生拖放测试，独立浮层与应用/系统模型分区，以及实际组件的布局、弹窗互斥和小部件选择器支持判定。仅构建组件时可在配置命令中添加 `-DBUILD_TESTING=OFF`，省去 Python、Xvfb 与 libXtst 测试依赖。

## 预览与安装

先在独立窗口预览，配置使用临时目录，当前面板配置保持原样：

```bash
preview_config="$(mktemp -d /tmp/kde-sys-tray-preview.XXXXXX)"
QT_PLUGIN_PATH="$PWD/build/bin${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}" \
  XDG_CONFIG_HOME="$preview_config" \
  plasmawindowed local.plasma.windowstray
```

在当前 Wayland 会话中检查应用图标的收纳、移出、排序，以及内置组件的点击与菜单。正式面板验收还应覆盖实际使用的面板方向、缩放比例和多行布局。

准备好加入面板后，停止桌面壳、安装，再启动桌面壳。首次安装和后续更新均可使用此流程，无需注销；面板与桌面短暂消失，应用窗口继续运行：

```bash
systemctl --user stop plasma-plasmashell.service
sudo cmake --install build
systemctl --user start plasma-plasmashell.service
```

上述 Fedora 配置将安装一个独立文件：

```text
/usr/lib64/qt6/plugins/plasma/applets/local.plasma.windowstray.so
```

桌面壳恢复后，进入面板编辑模式，添加“折叠系统托盘”。可先保留官方托盘对比，再移除旧组件；两者同时存在时会同时显示应用图标。新组件拥有独立配置，原托盘配置继续保留。

回退时先从面板移除派生组件、添加官方“系统托盘”，再删除派生插件：

```bash
sudo rm /usr/lib64/qt6/plugins/plasma/applets/local.plasma.windowstray.so
```

更改安装前缀或发行版时，以 `build/install_manifest.txt` 中的实际安装路径为准。

## 实现边界

```mermaid
flowchart LR
    A[左键超过阈值] --> B[Qt 原生拖拽]
    B --> C[落点暂存变更]
    C --> D[原生拖拽结束]
    D --> E[校验当前应用与目标]
    E --> F[发布配置并刷新模型]
```

- **拖拽会话**：`src/qml/ApplicationDrag.qml` 使用持久对象承载原生 Drag，面板与弹出窗口通过同一会话及实例标识交换拖放数据。完成原生拖拽后才提交变更，避免源图标在拖拽栈内被销毁。
- **布局与容器**：`TrayModels.qml` 将应用和系统项目按类型及显隐状态分区，`PanelTrayGrid.qml` 保留官方面板网格尺寸规则；`ApplicationOverflow.qml` 独立承载应用入口与图标浮层，不读写官方弹窗的尺寸配置。
- **落点与鼠标**：`ApplicationDropArea.qml` 管理应用区与应用浮层的落点；`AbstractItem.qml` 仅为应用启用拖拽阈值、点击抑制和应用间排序。系统组件保留原有事件路径，官方隐藏列表不接受应用拖入。
- **写入边界**：`src/applicationtraycontroller.cpp` 从当前模型重新验证源与目标均为应用，应用或目标已退出时拒绝变更；显隐与顺序通过一次配置发布生效。
- **配置与顺序**：复用官方 `shownItems` / `hiddenItems`，新增 `applicationOrder`。排序记录保留暂时退出的应用；`SortedSystemTrayModel` 维护应用区顺序，系统组件继续使用官方排序规则。

详细验证记录归档于本地 `artifacts/`，该目录由 Git 忽略。
