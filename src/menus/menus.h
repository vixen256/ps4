#include "diva.h"
namespace exitMenu {
void init ();
} // namespace exitMenu
namespace pvSel {
void init ();
void hide ();
void unhide ();
extern std::map<i32, i32> survivalIndexIds;
} // namespace pvSel
namespace gallery {
void init ();
} // namespace gallery
namespace options {
void init ();
} // namespace options
namespace pause {
void init ();
} // namespace pause
namespace customize {
void init ();
} // namespace customize
namespace result {
void init ();
} // namespace result
namespace decoration {
void init ();
} // namespace decoration
namespace pvWatch {
void init ();
} // namespace pvWatch
namespace pvGame {
void init (bool);
void D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext);
} // namespace pvGame
namespace genericDialog {
void init ();
} // namespace genericDialog
namespace commonUi {
void init ();
} // namespace commonUi
namespace commonMenu {
void init ();
} // namespace commonMenu
namespace leaderboard {
void init ();
} // namespace leaderboard
namespace gamma {
void D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext);
void OnResize (IDXGISwapChain *SwapChain);
void OnFrame (IDXGISwapChain *SwapChain);
} // namespace gamma