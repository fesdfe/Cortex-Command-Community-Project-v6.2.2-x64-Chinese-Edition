#pragma once

#ifndef GUI_STANDALONE
#define GUIAssert(expression, description) RTEAssert(expression, description)
#define GUIAbort(description) RTEAbort(description)
#else
#define GUIAssert(expression, description) assert(expression)
#define GUIAbort(description) abort()
#endif

#pragma region Rectangle Structure
/// The GUIRect structure defines a rectangle by the coordinates of its upper-left and lower-right corners.
struct GUIRect {
	long left;
	long top;
	long right;
	long bottom;
};

/// Sets the bounds of a GUIRect.
/// @param pRect Pointer to the GUIRect.
/// @param left Position of top left corner on X axis.
/// @param top Position of top left corner on Y axis.
/// @param right Position of bottom right corner on X axis.
/// @param bottom Position of bottom right corner on Y axis.
inline void SetRect(GUIRect* rect, int left, int top, int right, int bottom) {
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
}
#pragma endregion

#ifndef GUI_STANDALONE
#include "RTETools.h"
#endif

#include "GUIInterface.h"
#include "GUIProperties.h"
#include "GUIInput.h"
#include "GUIFont.h"
#include "GUISkin.h"
#include "GUIPanel.h"
#include "GUIManager.h"
#include "GUIUtil.h"
#include "GUIControl.h"
#include "GUIEvent.h"
#include "GUIControlFactory.h"
#include "GUIControlManager.h"

// ��������
#include "CNtools.h"

#ifndef GUI_STANDALONE
#include "GUISound.h"
#endif
