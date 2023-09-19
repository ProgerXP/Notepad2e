// [2e]: Back/Forward caret navigation hotkeys (Ctrl+Alt/Shift+O) #360
#ifndef POSITIONHISTORY_H
#define POSITIONHISTORY_H

namespace Scintilla {

	class PositionHistory {
		std::vector<SelectionRange> positions;
		std::vector<SelectionRange> redoPositions;

	public:
		void Clear() noexcept {
			positions.clear();
			redoPositions.clear();
		}
		bool CanUndo() const noexcept {
			return positions.size() > 0;
		}
		bool CanRedo() const noexcept {
			return redoPositions.size() > 0;
		}
		void PushPosition(const SelectionRange& sr) noexcept {
			if (!positions.empty()
				&& (positions.back().Length() > 0)
				&& (positions.back().Contains(sr.anchor) || positions.back().Contains(sr.caret)))
				positions.pop_back();
			if (positions.empty() || !(positions.back() == sr))
				positions.push_back(sr);
			redoPositions.clear();
		}
		SelectionRange UndoPosition() noexcept {
			if (!CanUndo())
				return {};
			redoPositions.push_back(positions.back());
			positions.pop_back();
			return (positions.size() > 0) ? positions.back() : redoPositions.back();
		}
		SelectionRange RedoPosition() noexcept {
			if (!CanRedo())
				return {};
			positions.push_back(redoPositions.back());
			redoPositions.pop_back();
			return positions.back();
		}
	};

}

#endif
