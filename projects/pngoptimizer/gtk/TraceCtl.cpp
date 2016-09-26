/////////////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizer application
// Copyright (C) Hadrien Nilsson - psydk.org
// For conditions of distribution and use, see copyright notice in PngOptimizer.h
/////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TraceCtl.h"

TraceCtl::TraceCtl()
{
	m_drawingArea = nullptr;

	m_maxLineWidthPx = 0;
	m_maxLineWidthWhen = 0;
	m_maxHeightPx = 0;
}

bool TraceCtl::Create(Window* parent, const String& welcomeMsg)
{
	m_welcomeMsg = welcomeMsg;

	auto parentHandle = parent->GetHandle();
	auto scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(parentHandle), scrolledWindow);

    m_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(m_drawingArea, 10, 10);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), m_drawingArea);

	//gtk_widget_set_double_buffered(m_drawingArea, false);
    g_signal_connect(m_drawingArea, "draw", G_CALLBACK(OnDrawStatic), this);
    
	m_handle = scrolledWindow;
    //auto view = gtk_text_view_new();
    //gtk_container_add(GTK_CONTAINER(scrolledWindow), view);
    return true;
}

gboolean TraceCtl::OnDrawStatic(GtkWidget* widget, cairo_t* cr, gpointer data)
{
	TraceCtl* that = reinterpret_cast<TraceCtl*>(data);
	return that->OnDraw(widget, cr);
}

int TraceCtl::DrawLine(const TextLine& line, cairo_t* cr, bool computeOnly,
                       int y)
{
	cairo_text_extents_t extents;
	GdkRGBA gc;
	gc.alpha = 1.0;

	int penX = 4;
	cairo_move_to(cr, penX, y);

	const TextPiece* pPiece = line.first;
	while( pPiece )
	{
		if( computeOnly )
		{
			cairo_text_extents(cr, pPiece->psz, &extents);
			penX += extents.x_advance;
		}
		else
		{
			const Color& col = pPiece->col;
			gc.red = double(col.r) / 255.0;
			gc.green = double(col.g) / 255.0;
			gc.blue = double(col.b) / 255.0;
			gdk_cairo_set_source_rgba(cr, &gc);
			cairo_show_text(cr, pPiece->psz);
		}
		pPiece = pPiece->next;
	}
	return penX;
}

// Informs GTK of the new size of the document
void TraceCtl::SyncDocumentMetrics(cairo_t* cr, int lineHeight)
{
	int lineCount = m_lines.GetSize();

	bool refresh = false;

	// Do not count last empty line
	if( lineCount > 0 && !m_lines[lineCount-1].first )
	{
		lineCount--;
	}

	// Width
	for(; m_maxLineWidthWhen < lineCount; ++m_maxLineWidthWhen)
	{
		int width = DrawLine(m_lines[m_maxLineWidthWhen], cr, true, 0);
		if( width > m_maxLineWidthPx )
		{
			m_maxLineWidthPx = width;
			refresh = true;
		}
	}

	// Height
	int totalHeight = lineCount * lineHeight + 1;
	if( totalHeight > m_maxHeightPx )
	{
		m_maxHeightPx = totalHeight;
		refresh = true;
	}

	if( refresh )
	{
		gtk_widget_set_size_request(m_drawingArea, m_maxLineWidthPx, m_maxHeightPx);
	}
}

bool TraceCtl::OnDraw(GtkWidget* widget, cairo_t* cr)
{
	// Drawing text seems to be very slow, check times
	uint64 time0 = System::GetTime64();
	GdkRGBA gc;
	gc.alpha = 1.0;

	char sz[1024];

	guint width = gtk_widget_get_allocated_width(widget);
	guint height = gtk_widget_get_allocated_height(widget);

	cairo_set_font_size(cr, 13);
	cairo_text_extents_t extents;

	if( m_lines.IsEmpty() )
	{
		Color col(0, 0, 200);
		gc.red = double(col.r) / 255.0;
		gc.green = double(col.g) / 255.0;
		gc.blue = double(col.b) / 255.0;
		gdk_cairo_set_source_rgba(cr, &gc);

		m_welcomeMsg.ToUtf8Z(sz);
		cairo_text_extents(cr, sz, &extents);

		cairo_move_to(cr, width/2 - extents.width/2, height/2);
		cairo_show_text(cr, sz);
		return false;
	}

	////////////////////////////
	gtk_style_context_get_color(gtk_widget_get_style_context(widget),
	                           GTK_STATE_FLAG_NORMAL,
	                           &gc);

	cairo_font_extents_t cfe;
	cairo_font_extents(cr, &cfe);

	//int marginLeft = 0;
	int marginTop = cfe.ascent;
	int lineHeight = cfe.height;
	
	//auto vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_handle));
	//double val = gtk_adjustment_get_value(vadj);
	//gdk_cairo_set_source_rgba(cr, &gc);
	//cairo_move_to(cr, 0, 45);
	//char tmp[100];
	//sprintf(tmp, "<%lf>", val);
	//cairo_show_text(cr, tmp);
	//printf("vpos: %lf\n", val);

	int startLine = 0;
	int lineCount = m_lines.GetSize();

	uint64 time1 = System::GetTime64();
	for(int i = startLine; i < lineCount; ++i)
	{
		int y = marginTop + i * lineHeight;
		DrawLine(m_lines[i], cr, false, y);
	}
	uint64 time2 = System::GetTime64();

	SyncDocumentMetrics(cr, lineHeight);
	uint64 time3 = System::GetTime64();

/*
	printf("---- times ----\n");
	printf("%f\n", double(time0)/1000.0);
	printf("%f\n", double(time1)/1000.0);
	printf("%f\n", double(time2)/1000.0);
	printf("%f\n", double(time3)/1000.0);
*/
	//fflush(stdout);
	(void)time0;
	(void)time1;
	(void)time2;
	(void)time3;

	return false;
}

void TraceCtl::Clear()
{
	m_lines.Clear();
	m_maxLineWidthPx = 0;
	m_maxLineWidthWhen = 0;
	m_maxHeightPx = 0;
	gtk_widget_queue_draw(GTK_WIDGET(m_drawingArea));
}

TraceCtl::TextLine::~TextLine()
{
	auto current = first;
	while( current )
	{
		auto next = current->next;
		free(current);
		current = next;
	}
}

void TraceCtl::AddText(const chustd::String& text, Color col)
{
	int lineIndex = -1;
	if( m_lines.IsEmpty() )
	{
		lineIndex = m_lines.Add();
	}
	else
	{
		lineIndex = m_lines.GetSize() - 1;
	}

	TextLine& line = m_lines[lineIndex];

	int utf8Len = 0;
	char tmp[500];
	text.ToUtf8Z(tmp, &utf8Len);

	bool addLine = false;
	if( text.EndsWith("\n") )
	{
		addLine = true;

		utf8Len--;
		if( utf8Len >= 0 )
		{
			tmp[utf8Len] = 0;
		}
	}

	if( utf8Len > 0 )
	{
		TextPiece* pPiece = nullptr;
		int tpSize = sizeof(TextPiece) - sizeof(pPiece->psz) + utf8Len + 1;
		pPiece = (TextPiece*)malloc(tpSize);
		pPiece->next = nullptr;
		pPiece->col = col;
		memcpy(pPiece->psz, tmp, utf8Len + 1);

		if( !line.first )
		{
			line.first = pPiece;
			line.last = pPiece;
		}
		else
		{
			TextPiece* pOldLast = line.last;
			pOldLast->next = pPiece;
			line.last = pPiece;
		}
	}

	// Always add the line at the end to garanty the address of line does not change
	if( addLine )
	{
		m_lines.Add();
	}
	auto vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_handle));
	double val = gtk_adjustment_get_upper(vadj);
	gtk_adjustment_set_value(vadj, val);

	gtk_widget_queue_draw(GTK_WIDGET(m_drawingArea));
}

