#pragma once

typedef struct Color {
	int r, g, b;

	Color() { r = 0; g = 0; b = 0; }
	Color(int R, int G, int B) { r = R; g = G; b = B; }
	Color(COLORREF fromRef) { b = (fromRef >> 16) & 0xFF; g = (fromRef >> 8) & 0xFF; r = (fromRef & 0xFF); }

	Color operator+(const Color& second) { return Color(r + second.r, g + second.g, b + second.b); }
	Color operator-(const Color& second) { return Color(r - second.r, g - second.g, b - second.b); }
	Color operator*(int t) { return Color(r * t, g * t, b * t); }
	Color operator/(int t) { return Color(r / t, g / t, b / t); }

	COLORREF toRGB() {
		return (b << 16) | (g << 8) | r;
	}
	Color Lerp(Color toColor, int t, int p) {
		return *this + (toColor - *this) * t / p;
	}
};

void GradientRect(HDC hDC, const RECT* lprc, Color leftColor, Color rightColor) {
	int width = lprc->left - lprc->right;

	for (int x = lprc->right; x < lprc->left; ++x) {
		COLORREF curColor = leftColor.Lerp(rightColor, x - lprc->right, width).toRGB();

		for (int y = lprc->top; y < lprc->bottom; ++y) {
			SetPixel(hDC, x, y, curColor);
		}
	}
}

void GradientRect(HDC hDC, const RECT* lprc, Color leftColor, Color rightColor, Color topColor, Color bottomColor) {
	int width = lprc->left - lprc->right;
	int height = lprc->bottom - lprc->top;
	int middle = (width + height) / 2;

	for (int x = lprc->right; x < lprc->left; ++x) {
		Color xColor = leftColor.Lerp(rightColor, x - lprc->right, width);

		for (int y = lprc->top; y < lprc->bottom; ++y) {
			Color yColor = topColor.Lerp(bottomColor, y - lprc->top, height);
			
			SetPixel(hDC, x, y, xColor.Lerp(yColor, ((x - lprc->right) + (y - lprc->top)) >> 1, middle).toRGB());
		}
	}
}