uDesktopDuplication
===================

**uDesktopDuplication** is an Unity asset to use the realtime screen capture as `Texture2D` using Desktop Duplication API.


ScreenShot
----------

![uDesktopDuplication](https://raw.githubusercontent.com/wiki/hecomi/uDesktopDuplication/animation.gif)


Environment
-----------

- Windows 8 / 10


Install
-------

- Unity Package
  - Download the latest .unitypackage from [Release page](https://github.com/hecomi/uDesktopDuplication/releases).
- Git URL (UPM)
  - Add `https://github.com/hecomi/uDesktopDuplication.git#upm` to Package Manager.
- Scoped Registry (UPM)
  - Add a scoped registry to your project.
    - URL: `https://registry.npmjs.com`
    - Scope: `com.hecomi`
  - Install uDesktopDuplication in Package Manager.


Usage
-----
Attach `uDesktopDuplication/Texture` component to the target object, then its main texture will be replaced with the captured screen. Please see sample scenes for more details.
