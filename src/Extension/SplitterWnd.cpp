#include "SplitterWnd.h"
#include <assert.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "CommonUtils.h"
#include "Scintilla.h"
#include "ViewHelper.h"

const wchar_t UC_SPLITTER_INDICATOR[] = { L"SplitterIndicatorWnd" };
const wchar_t UC_SPLITTER[] = { L"SplitterWnd" };
const wchar_t SETTINGS[] = { L"SplitterSettings" };

UINT WM_SPLITTER_CHILDREN_COUNT = RegisterWindowMessage(L"WM_GET_SPLITTER_CHILDREN");
UINT WM_SPLITTER_CHILD_BY_INDEX = RegisterWindowMessage(L"WM_SPLITTER_CHILD_BY_INDEX");

#define HTSPLITTER_MOVE 25
const int SPLITTER_GRIP_SIZE = GetSystemMetrics(SM_CYVSCROLL) * 3 / 8; // = 37.5%
const int SPLITTER_MIN_PANE_SIZE = 20;

#define INIT_SELF() auto pSelf = CSplitterWindow::FromHWND(hWnd);

struct Rect : public RECT
{
  Rect() { left = right = top = bottom = 0; }
  Rect(const int _left, const int _top, const int _right, const int _bottom) {
    left = _left; top = _top; right = _right; bottom = _bottom;
  }
  bool operator==(const Rect& r) const {
    return left == r.left
      && right == r.right
      && top == r.top
      && bottom == r.bottom;
  }
  bool empty() const { return *this == Rect(); }
  int width() const { return right - left; }
  int height() const { return bottom - top; }
  void ClientToScreen(const HWND hwnd) {
    ::ClientToScreen(hwnd, (LPPOINT)this);
    ::ClientToScreen(hwnd, ((LPPOINT)this) + 1);
  }
  void ScreenToClient(const HWND hwnd) {
    ::ScreenToClient(hwnd, (LPPOINT)this);
    ::ScreenToClient(hwnd, ((LPPOINT)this) + 1);
  }
};

class CSplitterResizingIndicator
{
private:
  static ATOM s_classAtom;
  HWND m_hwnd = NULL;
public:
  CSplitterResizingIndicator()
  {
    if (!s_classAtom)
    {
      WNDCLASS wc = { 0, DefWindowProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, GetSysColorBrush(COLOR_GRAYTEXT), NULL, UC_SPLITTER_INDICATOR };
      s_classAtom = RegisterClass(&wc);
    }

    m_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, UC_SPLITTER_INDICATOR, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 128, LWA_ALPHA);
  }
  HWND GetHWND() const { return m_hwnd; }
  void SetParent(const HWND hwndParent)
  {
    SetWindowLong(m_hwnd, GWL_HWNDPARENT, (LONG)hwndParent);
  }
  void Show(const Rect& rc)
  {
    SetWindowPos(m_hwnd, NULL, rc.left, rc.top, rc.width(), rc.height(), SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
  }
  void Hide() { ShowWindow(m_hwnd, SW_HIDE); }
};

class CSplitterPane
{
private:
  int m_size = 0; // width or height
  HWND m_hwndChild = NULL;
  Rect m_rcChild;
  Rect m_rcGrip;
public:
  CSplitterPane(const HWND hwnd) : m_hwndChild(hwnd) {}

  HWND GetChildHWND() const { return m_hwndChild; }
  void SetChildHWND(const HWND hwnd) { m_hwndChild = hwnd; }
  int GetSize() const { return m_size; }
  void SetSize(const int size) { m_size = size; }
  int UpdateChild(const Rect& rc, const bool isHorizontalSplitter, const int paneOffset, const bool isLastPane, HDWP* lpHDWP) {
    if (isHorizontalSplitter)
    {
      m_rcChild = { rc.left + paneOffset, rc.top, rc.left + paneOffset + m_size - SPLITTER_GRIP_SIZE, rc.bottom };
      m_rcGrip = { m_rcChild.right, m_rcChild.top, m_rcChild.right + SPLITTER_GRIP_SIZE, m_rcChild.bottom };
    }
    else
    {
      m_rcChild = { rc.left, rc.top + paneOffset, rc.right, rc.top + paneOffset + m_size - SPLITTER_GRIP_SIZE };
      m_rcGrip = { m_rcChild.left, m_rcChild.bottom, m_rcChild.right, m_rcChild.bottom + SPLITTER_GRIP_SIZE };
    }
    if (isLastPane)
    {
      UnionRect(&m_rcChild, &m_rcChild, &m_rcGrip);
      m_rcGrip = {};
    }
    *lpHDWP = DeferWindowPos(*lpHDWP, m_hwndChild, NULL, m_rcChild.left, m_rcChild.top, m_rcChild.width(), m_rcChild.height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    return GetSize();
  }
  const Rect& GetRect() const { return m_rcChild; }
  const Rect& GetGripRect() const { return m_rcGrip; }
};

class CSplitterWindow
{
private:
  static ATOM s_classAtom;

  HWND m_hwnd = NULL;
  bool m_isHorizontal = true;
  CSplitterResizingIndicator m_resizingIndicator;
  int m_resizingDiff = 0;

  Rect m_rcUpdate;
  typedef std::vector<CSplitterPane> TPanes;
  TPanes m_panes;
  TPanes::iterator m_hotPane;
  int m_hotPaneSizeOriginal = 0;
  int m_nextPaneSizeOriginal = 0;
  bool m_paneResizingMode = false;
  POINT m_ptResizingStart;

  static void init()
  {
    if (!s_classAtom)
    {
      WNDCLASS wc = { 0, SplitterProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, GetSysColorBrush(COLOR_BTNFACE), NULL, UC_SPLITTER };
      s_classAtom = RegisterClass(&wc);
    }
  }

  void setHWND(const HWND hwnd) { m_hwnd = hwnd; }

  static LRESULT CALLBACK SplitterProc(HWND hWndSplitter, UINT Message, WPARAM wParam, LPARAM lParam);

  TPanes::const_iterator getChild(const int index) const {
    auto it = m_panes.cbegin();
    advance(it, index);
    return it;
  }

  TPanes::const_iterator getChild(const HWND hwnd) const {
    return std::find_if(m_panes.cbegin(), m_panes.cend(), [&hwnd](const CSplitterPane& pane) {
      return pane.GetChildHWND() == hwnd;
    });
  }

  TPanes::iterator getChild(const HWND hwnd) {
    return std::find_if(m_panes.begin(), m_panes.end(), [&hwnd](const CSplitterPane& pane) {
      return pane.GetChildHWND() == hwnd;
    });
  }

public:
  CSplitterWindow(const HWND hwndParent, bool isHorizontal) : m_isHorizontal(isHorizontal) {
    init();
    CreateWindowEx(WS_EX_NOACTIVATE, UC_SPLITTER, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, 0, hwndParent, NULL, GetModuleHandle(NULL), (LPVOID)this);
    m_resizingIndicator.SetParent(m_hwnd);
  }

  HWND GetHWND() const { return m_hwnd; }
  bool IsHorizontal() const { return m_isHorizontal; }
  void SetHorizontal(const bool horizontal) { m_isHorizontal = horizontal; }

  HWND GetChild(const int index) const {
    auto it = getChild(index);
    return (it != m_panes.cend()) ? it->GetChildHWND() : NULL;
  }

  void AddChild(const HWND hwndChild) {
    const HWND hwndPrev = !m_panes.empty() ? GetPrevSibling(m_panes.rbegin()->GetChildHWND()) : HWND_TOP;
    m_panes.emplace_back(hwndChild);
    SetParent(hwndChild, m_hwnd);
    SetWindowPos(hwndChild, hwndPrev, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOOWNERZORDER);
  }

  void SwitchChild(const HWND hwndChildFrom, const HWND hwndChildTo) {
    auto it = getChild(hwndChildFrom);
    if (it != m_panes.end())
    {
      it->SetChildHWND(hwndChildTo);
      SetParent(hwndChildTo, m_hwnd);
    }
  }

  void DeleteChild(const HWND hwndChild, const bool destroyWindow) {
    auto it = getChild(hwndChild);
    if (it != m_panes.end())
    {
      if (destroyWindow)
      {
        DestroyWindow(hwndChild);
      }
      m_panes.erase(it);
      ScalePanes();
    }
  }

  void DeleteChildren(const HWND hwndParent, const bool destroyWindow) {
    auto it = m_panes.begin();
    while (it != m_panes.end())
    {
      const auto hwndChild = it->GetChildHWND();
      if (destroyWindow)
      {
        DestroyWindow(hwndChild);
      }
      else
      {
        SetParent(hwndChild, hwndParent);
      }
      it = m_panes.erase(it);
    }
  }

  bool IsChild(const HWND hwnd) const {
    return getChild(hwnd) != m_panes.cend();
  }

  size_t PaneCount() const { return m_panes.size(); }

  void UpdatePanes() {
    GetClientRect(m_hwnd, &m_rcUpdate);
    const int paneCount = max(PaneCount(), 1);
    const int paneSize = (m_isHorizontal ? m_rcUpdate.width() : m_rcUpdate.height()) / paneCount;
    int paneOffset = 0;
    HDWP hWinPosInfo = BeginDeferWindowPos(m_panes.size());
    for (auto it = m_panes.begin(); it != m_panes.end(); ++it)
    {
      auto& pane = *it;
      const bool isLastPane = std::distance(it, m_panes.end()) == 1;
      pane.SetSize(paneSize);
      paneOffset += pane.UpdateChild(m_rcUpdate, m_isHorizontal, paneOffset, isLastPane, &hWinPosInfo);
    }
    EndDeferWindowPos(hWinPosInfo);
  }

  void ScalePanes() {
    if (m_rcUpdate.empty())
    {
      UpdatePanes();
      return;
    }

    Rect rc;
    GetClientRect(m_hwnd, &rc);
    const int denominator = (m_isHorizontal ? m_rcUpdate.width() : m_rcUpdate.height());
    const int numerator = (m_isHorizontal ? rc.width() : rc.height());
    int paneOffset = 0;
    HDWP hWinPosInfo = BeginDeferWindowPos(m_panes.size());
    for (auto it = m_panes.begin(); it != m_panes.end(); ++it)
    {
      auto& pane = *it;
      const bool isLastPane = std::distance(it, m_panes.end()) == 1;
      const int paneSize = isLastPane
        ? numerator - paneOffset
        : MulDiv(pane.GetSize(), numerator, denominator);
      pane.SetSize(paneSize);
      paneOffset += pane.UpdateChild(rc, m_isHorizontal, paneOffset, isLastPane, &hWinPosInfo);
    }
    EndDeferWindowPos(hWinPosInfo);

    m_rcUpdate = rc;
  }

  void SetHotPane(const TPanes::iterator itPane) {
    m_hotPane = itPane;
    if (std::distance(m_hotPane, m_panes.end()) > 1)
    {
      m_hotPaneSizeOriginal = m_hotPane->GetSize();
      m_nextPaneSizeOriginal = (m_hotPane + 1)->GetSize();
    }
    else
    {
      m_hotPaneSizeOriginal = m_nextPaneSizeOriginal = 0;
      m_resizingDiff = 0;
    }
  }

  bool IsPaneResizing() const {
    return m_paneResizingMode;
  }

  void StartPaneResizing(const POINT& pt) {
    if ((m_hotPane != m_panes.end())
      && (std::distance(m_hotPane, m_panes.end()) > 1))
    {
      const auto& paneHot = *m_hotPane;
      const auto& paneNext = *(m_hotPane + 1);
      Rect rcSizing = paneHot.GetRect();
      if (m_isHorizontal)
      {
        rcSizing.left += SPLITTER_MIN_PANE_SIZE;
        rcSizing.right = paneNext.GetRect().right - SPLITTER_MIN_PANE_SIZE;
      }
      else
      {
        rcSizing.top += SPLITTER_MIN_PANE_SIZE;
        rcSizing.bottom = paneNext.GetRect().bottom - SPLITTER_MIN_PANE_SIZE;
      }
      rcSizing.ClientToScreen(m_hwnd);

      m_ptResizingStart = pt;
      m_paneResizingMode = true;
      ClipCursor(&rcSizing);
      SetCapture(m_hwnd);
    }
  }

  void ProcessPaneResizing(const POINT& pt) {
    m_resizingDiff = (m_isHorizontal ? m_ptResizingStart.x - pt.x : m_ptResizingStart.y - pt.y);
    if (m_paneResizingMode
      && (m_hotPane != m_panes.cend())
      && (std::distance(m_hotPane, m_panes.end()) >= 1))
    {
      Rect rc = m_hotPane->GetGripRect();
      if (m_isHorizontal)
      {
        rc.left -= m_resizingDiff;
        rc.right -= m_resizingDiff;
      }
      else
      {
        rc.top -= m_resizingDiff;
        rc.bottom -= m_resizingDiff;
      }
      rc.ClientToScreen(m_hwnd);
      m_resizingIndicator.Show(rc);
    }
  }

  void StopPaneSizing() {
    m_resizingIndicator.Hide();
    m_paneResizingMode = false;
    ClipCursor(NULL);
    ReleaseCapture();
    if (std::distance(m_hotPane, m_panes.end()) > 1)
    {
      auto& paneHot = *m_hotPane;
      auto& paneNext = *(m_hotPane + 1);
      paneHot.SetSize(m_hotPaneSizeOriginal - m_resizingDiff);
      paneNext.SetSize(m_nextPaneSizeOriginal + m_resizingDiff);
      int paneOffset = 0;
      for (auto it = m_panes.begin(); it != m_hotPane; ++it)
      {
        paneOffset += it->GetSize();
      }
      Rect rc;
      GetClientRect(m_hwnd, &rc);
      HDWP hWinPosInfo = BeginDeferWindowPos(m_panes.size());
      for (auto it = m_hotPane; it != m_panes.end(); ++it)
      {
        paneOffset += it->UpdateChild(rc, m_isHorizontal, paneOffset, std::distance(it, m_panes.end()) == 1, &hWinPosInfo);
      }
      EndDeferWindowPos(hWinPosInfo);
    }
    SetHotPane(m_panes.end());
  }

  void ProcessDoubleClick() {
    if (std::distance(m_hotPane, m_panes.end()) > 1)
    {
      auto it = m_hotPane;
      auto& pane1 = *it;
      auto& pane2 = *(++it);
      const auto paneAverageSize = (pane1.GetSize() + pane2.GetSize()) / 2;

      pane1.SetSize(paneAverageSize);
      pane2.SetSize(paneAverageSize);
      ScalePanes();

      SetHotPane(m_panes.end());
      StopPaneSizing();
    }
  }

  static bool IsValidHWND(const HWND hwnd) {
    WCHAR szClassName[64] = { 0 };
    return GetClassName(hwnd, szClassName, sizeof(szClassName) / sizeof(WCHAR)) && (wcscmp(szClassName, UC_SPLITTER) == 0);
  }

  static void AttachToHWND(const HWND hwnd, CSplitterWindow* pSplitter) {
    SetProp(hwnd, SETTINGS, (HANDLE)pSplitter);
  }

  static CSplitterWindow* FromHWND(const HWND hwnd) {
    return (CSplitterWindow*)GetProp(hwnd, SETTINGS);
  }
};

ATOM CSplitterWindow::s_classAtom = 0;
ATOM CSplitterResizingIndicator::s_classAtom = 0;

BOOL IsSplitterWnd(const HWND hwnd)
{
  return CSplitterWindow::IsValidHWND(hwnd);
}

HWND GetTopSplitterWnd(HWND hwnd)
{
  HWND hwndSplitter = hwnd;
  hwnd = GetParent(hwnd);
  while (IsSplitterWnd(hwnd))
  {
    hwndSplitter = hwnd;
    hwnd = GetParent(hwnd);
  }
  return hwndSplitter;
}

LRESULT CALLBACK CSplitterWindow::SplitterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  INIT_SELF();
  switch (uMsg)
  {
  case WM_CREATE:
    {
      LPCREATESTRUCT lpCreate = (LPCREATESTRUCT)lParam;
      pSelf = (CSplitterWindow*)lpCreate->lpCreateParams;
      pSelf->setHWND(hWnd);
      CSplitterWindow::AttachToHWND(hWnd, pSelf);
    }
    return 0;
  case WM_NCHITTEST:
    {
      const LRESULT lRes = DefWindowProc(hWnd, uMsg, wParam, lParam);
      POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      ScreenToClient(hWnd, &ptMouse);
      if (lRes == HTCLIENT)
      {
        auto it = std::find_if(pSelf->m_panes.begin(), pSelf->m_panes.end(), [&ptMouse](const CSplitterPane& pane){
            return PtInRect(&pane.GetGripRect(), ptMouse);
        });
        pSelf->SetHotPane(it);
        if (it != pSelf->m_panes.cend())
        {
          return HTSPLITTER_MOVE;
        }
      }
      return lRes;
    }
    break;
  case WM_SETCURSOR:
    if (LOWORD(lParam) == HTSPLITTER_MOVE)
    {
      SetCursor(LoadCursor(NULL, pSelf->IsHorizontal() ? IDC_SIZEWE : IDC_SIZENS));
      return TRUE;
    }
    break;
  case WM_NCLBUTTONDOWN:
    if (wParam == HTSPLITTER_MOVE)
    {
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      ScreenToClient(hWnd, &pt);
      pSelf->StartPaneResizing(pt);
      return 0;
    }
    break;
  case WM_NCLBUTTONDBLCLK:
    if (wParam == HTSPLITTER_MOVE) 
    {
      pSelf->ProcessDoubleClick();
      return 0;
    }
    break;
  case WM_MOUSEMOVE:
    if (pSelf->IsPaneResizing())
    {
      const POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      pSelf->ProcessPaneResizing(pt);
    }
    break;
  case WM_LBUTTONUP:
    if (pSelf->IsPaneResizing())
    {
      pSelf->StopPaneSizing();
      return 0;
    }
    break;
  case WM_WINDOWPOSCHANGED:
    pSelf->ScalePanes();
    break;
  case WM_NOTIFY:
  case WM_CONTEXTMENU:
    if (pSelf->IsChild((uMsg == WM_NOTIFY)
            ? ((LPNMHDR)lParam)->hwndFrom
            : (HWND)wParam))
    {
      if (uMsg == WM_NOTIFY)
      {
        LPNMHDR lpNMHDR = (LPNMHDR)lParam;
        if (n2e_IsScintillaWindow(lpNMHDR->hwndFrom) && (lpNMHDR->code == SCN_FOCUSIN))
        {
          n2e_SetActiveEdit(lpNMHDR->hwndFrom);
        }
      }
      const HWND hWndTarget = GetAncestor(hWnd, GA_ROOTOWNER);
      return SendMessage(hWndTarget, uMsg, wParam, lParam);
    }
    break;
  default:
    if (uMsg == WM_SPLITTER_CHILDREN_COUNT)
    {
      int res = pSelf->PaneCount();
      for (const auto& pane : pSelf->m_panes)
      {
        const HWND hwndChild = pane.GetChildHWND();
        if (IsSplitterWnd(hwndChild))
        {
          res += SendMessage(hwndChild, WM_SPLITTER_CHILDREN_COUNT, 0, 0) - 1;
        }
      }
      return res;
    }
    else if (uMsg == WM_SPLITTER_CHILD_BY_INDEX)
    {
      HWND res = NULL;
      int index = (int)wParam;
      for (const auto& pane : pSelf->m_panes)
      {
        const HWND hwndChild = pane.GetChildHWND();
        if (IsSplitterWnd(hwndChild))
        {
          const int innerChildren = SendMessage(hwndChild, WM_SPLITTER_CHILDREN_COUNT, 0, 0);
          if (index < innerChildren)
          {
            return (LRESULT)SendMessage(hwndChild, WM_SPLITTER_CHILD_BY_INDEX, index, 0);
          }
          else
          {
            index -= innerChildren;
            continue;
          }
        }
        if (index == 0)
        {
          res = hwndChild;
          break;
        }
        --index;
      }
      return (LRESULT)res;
    }
    break;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CreateSplitterWnd(const HWND hwndParent, const HWND hwndChild1, const HWND hwndChild2, const BOOL bHorizontal)
{
  auto pSplitter = new CSplitterWindow(hwndParent, bHorizontal);
  if (!pSplitter)
  {
    assert(0);
    return NULL;
  }
  pSplitter->AddChild(hwndChild1);
  pSplitter->AddChild(hwndChild2);
  pSplitter->UpdatePanes();
  return pSplitter->GetHWND();
}

HWND AddSplitterChild(HWND hwndParent, const HWND hwndChildActive, const HWND hwndChild, const BOOL bHorizontal)
{
  HWND hwndSplitter = hwndParent;
  CSplitterWindow* pSplitter = CSplitterWindow::FromHWND(hwndParent);
  if (!pSplitter)
  {
    assert(false);
    return NULL;
  }

  if (pSplitter->IsHorizontal() == (bool)bHorizontal)
  {
    pSplitter->AddChild(hwndChild);
    pSplitter->UpdatePanes();
  }
  else
  {
    HWND hwndPrev = GetPrevSibling(hwndChildActive);
    hwndSplitter = CreateSplitterWnd(pSplitter->GetHWND(), hwndChildActive, hwndChild, bHorizontal);
    SetWindowPos(hwndSplitter, hwndPrev, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
    pSplitter->SwitchChild(hwndChildActive, hwndSplitter);
    pSplitter->ScalePanes();
  }

  hwndParent = hwndSplitter;
  while (IsSplitterWnd(hwndParent))
  {
    hwndSplitter = hwndParent;
    hwndParent = GetParent(hwndParent);
  }
  return hwndSplitter;
}

bool ChildToTopSplitter(HWND hwndChild, CSplitterWindow* pSplitter, CSplitterWindow* pTopSplitter)
{
  if (pTopSplitter)
  {
    pSplitter->DeleteChild(hwndChild, false);
    SetParent(hwndChild, pTopSplitter->GetHWND());
    pTopSplitter->SwitchChild(pSplitter->GetHWND(), hwndChild);
    pTopSplitter->ScalePanes();
    return true;
  }
  else
  {
    return false;
  }
}

void DeleteSplitterChild(HWND hwndChild, HWND hwndParentForLast, HWND* phwndEditParent)
{
  CSplitterWindow* pSplitter = CSplitterWindow::FromHWND(GetParent(hwndChild));
  if (!pSplitter)
  {
    return;
  }
  pSplitter->DeleteChild(hwndChild, true);
  if (pSplitter->PaneCount() == 1)
  {
    const HWND hwndLastChild = pSplitter->GetChild(0);
    CSplitterWindow* pChildSplitter = CSplitterWindow::FromHWND(hwndLastChild);
    if (pChildSplitter)
    {
      const bool isHorizontal = pChildSplitter->IsHorizontal();
      for (size_t i = 0; i < pChildSplitter->PaneCount(); ++i)
      {
        const auto hwndChild = pChildSplitter->GetChild(i);
        pSplitter->AddChild(hwndChild);
      }
      pSplitter->DeleteChild(hwndLastChild, true);
      pSplitter->SetHorizontal(isHorizontal);
      pSplitter->UpdatePanes();
      return;
    }
    else
    {
      CSplitterWindow* pTopSplitter = CSplitterWindow::FromHWND(GetParent(pSplitter->GetHWND()));
      if (!ChildToTopSplitter(hwndLastChild, pSplitter, pTopSplitter))
      {
        SetParent(hwndLastChild, hwndLastChild == hwndParentForLast ? GetParent(pSplitter->GetHWND()) : hwndParentForLast);
        *phwndEditParent = hwndParentForLast;
      }
    }
    DestroyWindow(pSplitter->GetHWND());
    delete pSplitter;
  }
  else
  {
    pSplitter->ScalePanes();
  }
}
