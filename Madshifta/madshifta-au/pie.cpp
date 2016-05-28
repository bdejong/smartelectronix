TTobyGauge::TTobyGauge()
{
	FBorderStyle = bsSingle;
	FForeColor = clBlack;
	FBackColor = clWhite;
	FBorderColor = clWhite;
	Width = 100;
	Height = 100;
}

void TTobyGauge::PaintAsPie(TBitmap AnImage, TRect PaintRect)
{
	if (FBorderStyle == bsSingle)
	{
		Inc(PaintRect.Width);
		Inc(PaintRect.Height);
	}
	with (AnImage.Canvas) do
	{
		Brush.Color = Color;
		FillRect(PaintRect);
		Brush.Color = BackColor;
		Pen.Color = BorderColor;
		Pen.Width = 1;
		Ellipse(PaintRect.Left, PaintRect.Top, PaintRect.Width, PaintRect.Height);

		if ( ((PercentDone <= 0) && (!fInvPie)) || ((PercentDone >= 100) && (fInvPie)) )
			return;

		Brush.Color = ForeColor;
		int MiddleX = PaintRect.Width / 2;
		int MiddleY = PaintRect.Height / 2;

		double Angle;
		if (fInvPie)
			Angle = (Pi * (((100.0-PercentDone) / 50.0) + 0.5))
		else
			Angle = (Pi * ((PercentDone / 50.0) + 0.5));
		Pie(PaintRect.Left, PaintRect.Top, PaintRect.Width, PaintRect.Height,
			MiddleX * (1.0 - cos(Angle)), MiddleY * (1.0 - sin(Angle)), MiddleX, 0);
	}
}
