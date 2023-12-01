// Scintilla source code edit control
/** @file EditModel.h
 ** Defines the editor state that must be visible to EditorView.
 **/
// Copyright 1998-2014 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EDITMODEL_H
#define EDITMODEL_H

namespace Scintilla {

/**
*/
class Caret {
public:
	bool active;
	bool on;
	int period;

	Caret();
};

class EditModel {
public:
	bool inOverstrike;
	int xOffset;		///< Horizontal scrolled amount in pixels
	bool trackLineWidth;

	SpecialRepresentations reprs;
	Caret caret;
	SelectionPosition posDrag;
	Sci::Position braces[2];
	int bracesMatchStyle;
	int highlightGuideColumn;
	Selection sel;
	bool primarySelection;

	enum IMEInteraction { imeWindowed, imeInline } imeInteraction;
	enum class CharacterSource { directInput, tentativeInput, imeResult };

	int foldFlags;
	int foldDisplayTextStyle;
	UniqueString defaultFoldDisplayText;
	std::unique_ptr<IContractionState> pcs;
	// Hotspot support
	Range hotspot;
	Sci::Position hoverIndicatorPos;

	// Wrapping support
	int wrapWidth;

	Document *pdoc;

	EditModel();
	// Deleted so EditModel objects can not be copied.
	EditModel(const EditModel &) = delete;
	EditModel(EditModel &&) = delete;
	EditModel &operator=(const EditModel &) = delete;
	EditModel &operator=(EditModel &&) = delete;
	virtual ~EditModel();
	virtual Sci::Line TopLineOfMain() const = 0;
	virtual Point GetVisibleOriginInMain() const = 0;
	virtual Sci::Line LinesOnScreen() const = 0;
	virtual Range GetHotSpotRange() const noexcept = 0;
	void SetDefaultFoldDisplayText(const char *text);
	const char *GetDefaultFoldDisplayText() const noexcept;
	const char *GetFoldDisplayText(Sci::Line lineDoc) const;

	// [2e]: Find first/last match indication #388
	Sci::Line firstIndicatedLine = -1, lastIndicatedLine = -1;
	Sci::Line beforeIndicatedLine = -1, afterIndicatedLine = -1;
	virtual void SetIndicatedLines(
		const Sci::Line line1, const bool isFirst,
		const Sci::Line line2, const bool isLast) {
			firstIndicatedLine = isFirst ? line1 : -1;
			beforeIndicatedLine = isFirst ? -1 : line1;
			lastIndicatedLine = isLast ? line2 : -1;
			afterIndicatedLine = isLast ? -1 : line2;
	}
};

}

#endif
