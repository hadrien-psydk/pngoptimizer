/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in License.txt
/////////////////////////////////////////////////////////////////////////////////////

#ifndef PO_TRACECTL_H
#define PO_TRACECTL_H

#include "Widgets.h"

class TraceCtl : public Window
{
public:
	TraceCtl();

	bool Create(Window* parent, const String& welcomeMsg);
	void Clear();

	void AddText(const chustd::String& text, Color col = Color::Black);

private:
	GtkWidget* m_drawingArea;
	String     m_welcomeMsg;

	struct TextPiece
	{
		TextPiece* next;
		Color col;
		char psz[8];
	};
	struct TextLine
	{
		TextPiece* first;
		TextPiece* last;

		TextLine() : first(nullptr), last(nullptr) {}
		~TextLine();
	};
	Array<TextLine> m_lines;

	// Variables used to recompute the max line width
	int m_maxLineWidthPx;
	int m_maxLineWidthWhen; // m_lines size when maxLineWidthPx was computed
	int m_maxHeightPx;

private:
	static gboolean OnDrawStatic(GtkWidget*, cairo_t*, gpointer);
	bool OnDraw(GtkWidget* widget, cairo_t* cr);
	int  DrawLine(const TextLine& line, cairo_t* cr, bool computeOnly, int y);
	void SyncDocumentMetrics(cairo_t* cr, int lineHeight);
};

#endif
