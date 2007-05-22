//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/symbol/viewer.hpp>
#include <gui/util.hpp> // clearDC_black

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Simple rendering

Image render_symbol(const SymbolP& symbol, double border_radius, int size) {
	SymbolViewer viewer(symbol, border_radius);
	Bitmap bmp(size, size);
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	clearDC(dc, Color(0,128,0));
	viewer.rotation.setZoom(size);
	viewer.draw(dc);
	dc.SelectObject(wxNullBitmap);
	return bmp.ConvertToImage();
}

// ----------------------------------------------------------------------------- : Constructor

SymbolViewer::SymbolViewer(const SymbolP& symbol, double border_radius)
	: border_radius(border_radius)
	, rotation(0, RealRect(0,0,500,500), 500)
{
	setSymbol(symbol);
}

// ----------------------------------------------------------------------------- : Drawing

typedef shared_ptr<wxMemoryDC> MemoryDCP;

// Return a temporary DC with the same size as the parameter
MemoryDCP getTempDC(DC& dc) {
	wxSize s = dc.GetSize();
	Bitmap buffer(s.GetWidth(), s.GetHeight(), 1);
	MemoryDCP newDC(new wxMemoryDC);
	newDC->SelectObject(buffer);
	clearDC_black(*newDC);
	return newDC;
}

// Combine the temporary DCs used in the drawing with the main dc
void combineBuffers(DC& dc, DC* borders, DC* interior) {
	wxSize s = dc.GetSize();
	if (borders)  dc.Blit(0, 0, s.GetWidth(), s.GetHeight(), borders,  0, 0, wxOR);
	if (interior) dc.Blit(0, 0, s.GetWidth(), s.GetHeight(), interior, 0, 0, wxAND_INVERT);
}

void SymbolViewer::draw(DC& dc) {
	bool paintedSomething = false;
	bool buffersFilled    = false;
	// Temporary dcs
	MemoryDCP borderDC;
	MemoryDCP interiorDC;
	// Check if we can paint directly to the dc
	// This will fail if there are parts with combine == intersection
	FOR_EACH(p, symbol->parts) {
		if (p->combine == PART_INTERSECTION) {
			paintedSomething = true;
			break;
		}
	}
	// Draw all parts, in reverse order (bottom to top)
	FOR_EACH_REVERSE(p, symbol->parts) {
		const SymbolPart& part = *p;
		if (part.combine == PART_OVERLAP && buffersFilled) {
			// We will be overlapping some previous parts, write them to the screen
			combineBuffers(dc, borderDC.get(), interiorDC.get());
			// Clear the buffers
			buffersFilled = false;
			paintedSomething = true;
			wxSize s = dc.GetSize();
			if (borderDC) {
				borderDC->SetBrush(*wxBLACK_BRUSH);
				borderDC->SetPen(  *wxTRANSPARENT_PEN);
				borderDC->DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
			}
			interiorDC->SetBrush(*wxBLACK_BRUSH);
			interiorDC->DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
		}
		
		if (!paintedSomething) {
			// No need to buffer
			if (!interiorDC) interiorDC = getTempDC(dc);
			combineSymbolPart(part, dc, *interiorDC, true, false);
			buffersFilled = true;
		} else {
			if (!borderDC)    borderDC   = getTempDC(dc);
			if (!interiorDC)  interiorDC = getTempDC(dc);
			// Draw this part to the buffer
			combineSymbolPart(part, *borderDC, *interiorDC, false, false);
			buffersFilled = true;
		}
	}
	
	// Output the final parts from the buffer
	if (buffersFilled) {
		combineBuffers(dc, borderDC.get(), interiorDC.get());
	}
}

void SymbolViewer::highlightPart(DC& dc, const SymbolPart& part, HighlightStyle style) {
	// create point list
	vector<wxPoint> points;
	size_t size = part.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segment_subdivide(*part.getPoint((int)i), *part.getPoint((int)i+1), rotation, points);
	}
	// draw
	if (style == HIGHLIGHT_BORDER) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen  (wxPen(Color(255,0,0), 2));
		dc.DrawPolygon((int)points.size(), &points[0]);
	} else {
		dc.SetLogicalFunction(wxOR);
		dc.SetBrush(Color(0,0,64));
		dc.SetPen  (*wxTRANSPARENT_PEN);
		dc.DrawPolygon((int)points.size(), &points[0]);
		if (part.combine == PART_SUBTRACT || part.combine == PART_BORDER) {
			dc.SetLogicalFunction(wxAND);
			dc.SetBrush(Color(191,191,255));
			dc.DrawPolygon((int)points.size(), &points[0]);
		}
		dc.SetLogicalFunction(wxCOPY);
	}
}


void SymbolViewer::combineSymbolPart(const SymbolPart& part, DC& border, DC& interior, bool directB, bool directI) {
	// what color should the interior be?
	// use black when drawing to the screen
	unsigned char interiorCol = directI ? 0 : 255;
	// how to draw depends on combining mode
	switch(part.combine) {
		case PART_OVERLAP:
		case PART_MERGE: {
			drawSymbolPart(part, &border, &interior, 255, interiorCol, directB, false);
			break;
		} case PART_SUBTRACT: {
			border.SetLogicalFunction(wxAND);
			drawSymbolPart(part, &border, &interior, 0, ~interiorCol, true, true);
			border.SetLogicalFunction(wxCOPY);
			break;
		} case PART_INTERSECTION: {
			MemoryDCP keepBorder   = getTempDC(border);
			MemoryDCP keepInterior = getTempDC(interior);
			drawSymbolPart(part, keepBorder.get(), keepInterior.get(), 255, 255, false, false);
			// combine the temporary dcs with the result using the AND operator
			wxSize s = border.GetSize();
			border  .Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepBorder  , 0, 0, wxAND);
			interior.Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepInterior, 0, 0, wxAND);
			break;
		} case PART_DIFFERENCE: {
			interior.SetLogicalFunction(wxXOR);
			drawSymbolPart(part, &border, &interior, 0, ~interiorCol, directB, true);
			interior.SetLogicalFunction(wxCOPY);
			break;
		} case PART_BORDER: {
			// draw border as interior
			drawSymbolPart(part, nullptr, &border, 0, 255, false, false);
			break;
		}
	}
}

void SymbolViewer::drawSymbolPart(const SymbolPart& part, DC* border, DC* interior, unsigned char borderCol, unsigned char interiorCol, bool directB, bool clear) {
	// create point list
	vector<wxPoint> points;
	size_t size = part.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segment_subdivide(*part.getPoint((int)i), *part.getPoint((int)i+1), rotation, points);
	}
	// draw border
	if (border) {
		// white/black
		border->SetBrush(Color(borderCol, borderCol^(directB ? 128 : 0), borderCol));
		border->SetPen(wxPen(*wxWHITE, (int) rotation.trS(border_radius)));
		border->DrawPolygon((int)points.size(), &points[0]);

		if (clear) {
			border->SetPen(*wxTRANSPARENT_PEN);
			border->SetBrush(Color(0, (directB ? 128 : 0), 0));

			int func = border->GetLogicalFunction();
			border->SetLogicalFunction(wxCOPY);
			border->DrawPolygon((int)points.size(), &points[0]);
			border->SetLogicalFunction(func);
		}
	}
	// draw interior
	if (interior) {
		interior->SetBrush(Color(interiorCol,interiorCol,interiorCol));
		interior->SetPen(*wxTRANSPARENT_PEN);
		interior->DrawPolygon((int)points.size(), &points[0]);
	}
}
