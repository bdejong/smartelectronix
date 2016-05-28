unit tobyGauges;

interface

uses SysUtils, Windows, Messages, Classes, Graphics, Controls, Forms, StdCtrls;

type

  TTobyGaugeKind = (gkText, gkHorizontalBar, gkVerticalBar, gkPie, gkNeedle);

  TTobyGauge = class(TGraphicControl)
  private
    FMinValue: Longint;
    FMaxValue: Longint;
    FCurValue: Longint;
    FKind: TTobyGaugeKind;
    FShowText: Boolean;
    FBorderStyle: TBorderStyle;
    FForeColor: TColor;
    FBackColor: TColor;
    FBorderColor: TColor;
    fInvPie,fCenter:boolean;
    fTextXOffs,fTextYOffs:integer;
    procedure PaintBackground(AnImage: TBitmap);
    procedure PaintAsText(AnImage: TBitmap; PaintRect: TRect);
    procedure PaintAsNothing(AnImage: TBitmap; PaintRect: TRect);
    procedure PaintAsBar(AnImage: TBitmap; PaintRect: TRect);
    procedure PaintAsPie(AnImage: TBitmap; PaintRect: TRect);
    procedure PaintAsNeedle(AnImage: TBitmap; PaintRect: TRect);
    procedure SeTTobyGaugeKind(Value: TTobyGaugeKind);
    procedure SetShowText(Value: Boolean);
    procedure SetCenter(Value: Boolean);
    procedure SetInvPie(Value: Boolean);
    procedure SetBorderStyle(Value: TBorderStyle);
    procedure SetForeColor(Value: TColor);
    procedure SetBackColor(Value: TColor);
    procedure SetBorderColor(Value: TColor);
    procedure SetTextXOffs(Value: Longint);
    procedure SetTextYOffs(Value: Longint);
    procedure SetMinValue(Value: Longint);
    procedure SetMaxValue(Value: Longint);
    procedure SetProgress(Value: Longint);
    function GetPercentDone: Longint;
  protected
    procedure Paint; override;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
  public
    formatstring:string;
    constructor Create(AOwner: TComponent); override;
    procedure AddProgress(Value: Longint);
    property PercentDone: Longint read GetPercentDone;
  published
    property Align;
    property Color;
    property Enabled;

    property InvertPie:boolean read fInvPie write SetInvPie default false;

    property Kind: TTobyGaugeKind read FKind write SetTobyGaugeKind default gkHorizontalBar;
    property ShowText: Boolean read FShowText write SetShowText default True;
    property Font;

    property TextXOffset: integer read fTextXOffs write SetTextXOffs default 0;
    property TextYOffset: integer read fTextYOffs write SetTextYOffs default 0;

    property BorderStyle: TBorderStyle read FBorderStyle write SetBorderStyle default bsSingle;
    property ForeColor: TColor read FForeColor write SetForeColor default clBlack;
    property BackColor: TColor read FBackColor write SetBackColor default clWhite;
    property BorderColor: TColor read FBorderColor write SetBorderColor default clWhite;
    property MinValue: Longint read FMinValue write SetMinValue default 0;
    property MaxValue: Longint read FMaxValue write SetMaxValue default 100;
    property ParentColor;
    property CenterOrigin: boolean read fCenter write SetCenter default false;
    property ParentFont;
    property ParentShowHint;
    property PopupMenu;
    property Progress: Longint read FCurValue write SetProgress;
    property ShowHint;
    property Visible;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
  end;

procedure Register;

implementation

uses Consts;

type
  TBltBitmap = class(TBitmap)
    procedure MakeLike(ATemplate: TBitmap);
  end;

{ TBltBitmap }

procedure TBltBitmap.MakeLike(ATemplate: TBitmap);
begin
  Width := ATemplate.Width;
  Height := ATemplate.Height;
  Canvas.Brush.Color := clWindowFrame;
  Canvas.Brush.Style := bsSolid;
  Canvas.FillRect(Rect(0, 0, Width, Height));
end;

{ This function solves for x in the equation "x is y% of z". }
function SolveForX(Y, Z: Longint): Longint;
begin
  Result := round( Z * (Y * 0.01) );//tt
end;

{ This function solves for y in the equation "x is y% of z". }
function SolveForY(X, Z: Longint): Longint;
begin
  if Z = 0 then Result := 0
  else Result := round( (X * 100.0) / Z ); //t
end;

{ TTobyGauge }

procedure TTobyGauge.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); 
var MyMouseDown: TMouseEvent;
begin
 inherited MouseDown(Button, Shift, X, Y);
 MyMouseDown := OnMouseDown;
 if Assigned(MyMouseDown) then MyMouseDown(Self, Button, Shift, X, Y);
end;

constructor TTobyGauge.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  ControlStyle := ControlStyle + [csFramed, csOpaque];
  { default values }
  formatstring:='%d%%';
  FTextXOffs:=0;
  FTextYOffs:=0;
  FMinValue := 0;
  FMaxValue := 100;
  FCurValue := 0;
  FKind := gkHorizontalBar;
  FShowText := True;
  FBorderStyle := bsSingle;
  FForeColor := clBlack;
  FBackColor := clWhite;
  FBorderColor := clWhite;
  Width := 100;
  Height := 100;
end;

function TTobyGauge.GetPercentDone: Longint;
begin
  Result := SolveForY(FCurValue - FMinValue, FMaxValue - FMinValue);
end;

procedure TTobyGauge.Paint;
var
  TheImage: TBitmap;
  OverlayImage: TBltBitmap;
  PaintRect: TRect;
begin
  with Canvas do
  begin
    TheImage := TBitmap.Create;
    try
      TheImage.Height := Height;
      TheImage.Width := Width;
      PaintBackground(TheImage);
      PaintRect := ClientRect;
      if FBorderStyle = bsSingle then InflateRect(PaintRect, -1, -1);
      OverlayImage := TBltBitmap.Create;
      try
        OverlayImage.MakeLike(TheImage);
        PaintBackground(OverlayImage);
        case FKind of
          gkText: PaintAsNothing(OverlayImage, PaintRect);
          gkHorizontalBar, gkVerticalBar: PaintAsBar(OverlayImage, PaintRect);
          gkPie: PaintAsPie(OverlayImage, PaintRect);
          gkNeedle: PaintAsNeedle(OverlayImage, PaintRect);
        end;
        TheImage.Canvas.CopyMode := cmSrcInvert;
        TheImage.Canvas.Draw(0, 0, OverlayImage);
        TheImage.Canvas.CopyMode := cmSrcCopy;
        if ShowText then PaintAsText(TheImage, PaintRect);
      finally
        OverlayImage.Free;
      end;
      Canvas.CopyMode := cmSrcCopy;
      Canvas.Draw(0, 0, TheImage);
    finally
      TheImage.Destroy;
    end;
  end;
end;

procedure TTobyGauge.PaintBackground(AnImage: TBitmap);
var
  ARect: TRect;
begin
  with AnImage.Canvas do
  begin
    CopyMode := cmBlackness;
    ARect := Rect(0, 0, Width, Height);
    CopyRect(ARect, Animage.Canvas, ARect);
    CopyMode := cmSrcCopy;
  end;
end;

procedure TTobyGauge.PaintAsText(AnImage: TBitmap; PaintRect: TRect);
var
  S: string;
  X, Y: Integer;
  OverRect: TBltBitmap;
begin
  OverRect := TBltBitmap.Create;
  try
    OverRect.MakeLike(AnImage);
    PaintBackground(OverRect);
    if tag=99 then
     S := Format('%d', [round(-0.48*(PercentDone-50))])
    else
    if fcenter then
     S := Format(formatstring, [-2*(PercentDone-50)])
    else
     S := Format(formatstring, [PercentDone]);
    with OverRect.Canvas do
    begin
      Brush.Style := bsClear;
      Font := Self.Font;
//      Font.Color := clWhite;
      with PaintRect do
      begin
        X := (Right - Left + 1 - TextWidth(S)) div 2;
        Y := (Bottom - Top + 1 - TextHeight(S)) div 2;
      end;
//      TextRect(PaintRect, X, Y, S);
      TextOut(X+fTextXOffs, Y+fTextYOffs, S);
    end;
    AnImage.Canvas.CopyMode := cmSrcInvert;
    AnImage.Canvas.Draw(0, 0, OverRect);
  finally
    OverRect.Free;
  end;
end;

procedure TTobyGauge.PaintAsNothing(AnImage: TBitmap; PaintRect: TRect);
begin
  with AnImage do
  begin
    Canvas.Brush.Color := BackColor;
    Canvas.FillRect(PaintRect);
  end;
end;

procedure TTobyGauge.PaintAsBar(AnImage: TBitmap; PaintRect: TRect);
var
  FillSize: Longint;
  W, H: Integer;
begin
  W := PaintRect.Right - PaintRect.Left + 1;
  H := PaintRect.Bottom - PaintRect.Top + 1;
  with AnImage.Canvas do
  begin
    if borderstyle<>bsnone then
    begin
     Brush.Color := BorderColor;
     FillRect(PaintRect);
     W:=W-1;
     H:=H-1;
     PaintRect.Right:=PaintRect.Right-1;
     PaintRect.Left:=PaintRect.Left+1;
     PaintRect.Top:=PaintRect.Top+1;
     PaintRect.Bottom:=PaintRect.Bottom-1;
     Brush.Color := BackColor;
     FillRect(PaintRect);
    end else
    begin
     Brush.Color := BackColor;
     FillRect(PaintRect);
    end;
//    Pen.Color := BorderColor;
    Pen.Color := ForeColor;
    Pen.Width := 1;
    Brush.Color := ForeColor;
    case FKind of
      gkHorizontalBar:
        begin
         if not fCenter then
         begin
          FillSize := SolveForX(PercentDone, W);
          if FillSize > W then FillSize := W;
          if FillSize > 0 then FillRect(Rect(PaintRect.Left, PaintRect.Top,
            FillSize, H));
         end else
         begin
          if percentdone>50 then
          begin
           FillSize := SolveForX(2*(PercentDone-50), W div 2);
           if FillSize > W div 2 then FillSize := (W div 2);
           if FillSize >= 0 then
            FillRect(Rect(W div 2, PaintRect.Top, (W div 2)+FillSize, H));
          end else
          begin
           FillSize := SolveForX(2*PercentDone, W div 2);
           if FillSize > W div 2 then FillSize := (W div 2);
           if FillSize >= 0 then
            FillRect(Rect(FillSize, PaintRect.Top, (W div 2), H));
          end;
         end;
        end;
      gkVerticalBar:
        begin
         if not fCenter then
         begin
          FillSize := SolveForX(PercentDone, H);
          if FillSize >= H then FillSize := H;
          if FillSize > 0 then FillRect(Rect(PaintRect.Left, H-FillSize,
            W, H));
         end else
         begin
          if percentdone>50 then
          begin
           FillSize := SolveForX(2*(PercentDone-50), H div 2);
           if FillSize > H div 2 then FillSize := (H div 2);
           FillRect(Rect(PaintRect.Left, H div 2, W, (H div 2)+FillSize+1))
          end else
          begin
           FillSize := SolveForX(2*PercentDone, H div 2);
           if FillSize >= (H div 2)-1 then FillSize := (H div 2)-1;
           FillRect(Rect(PaintRect.Left, FillSize, W, H div 2));
          end;
         end;
        end;
    end;
  end;
end;

procedure TTobyGauge.PaintAsPie(AnImage: TBitmap; PaintRect: TRect);
var
  MiddleX, MiddleY: Integer;
  Angle: Double;
  W, H: Integer;
begin
  W := PaintRect.Right - PaintRect.Left;
  H := PaintRect.Bottom - PaintRect.Top;
  if FBorderStyle = bsSingle then
  begin
    Inc(W);
    Inc(H);
  end;
  with AnImage.Canvas do
  begin
    Brush.Color := Color;
    FillRect(PaintRect);
    Brush.Color := BackColor;
    Pen.Color := BorderColor;
    Pen.Width := 1;
    Ellipse(PaintRect.Left, PaintRect.Top, W, H);

    if ((PercentDone<=0)and(not fInvPie))or((PercentDone>=100)and(fInvPie)) then
     exit;
    begin
      Brush.Color := ForeColor;
      MiddleX := W div 2;
      MiddleY := H div 2;

      If fInvPie then
       Angle := (Pi * (((100-PercentDone) / 50) + 0.5))
      else
       Angle := (Pi * ((PercentDone / 50) + 0.5));
      Pie(PaintRect.Left, PaintRect.Top, W, H,
        Round(MiddleX * (1 - Cos(Angle))),
        Round(MiddleY * (1 - Sin(Angle))), MiddleX, 0);

    end;
  end;
end;

procedure TTobyGauge.PaintAsNeedle(AnImage: TBitmap; PaintRect: TRect);
var
  MiddleX: Integer;
  Angle: Double;
  X, Y, W, H: Integer;
begin
  with PaintRect do
  begin
    X := Left;
    Y := Top;
    W := Right - Left;
    H := Bottom - Top;
    if FBorderStyle = bsSingle then
    begin
      Inc(W);
      Inc(H);
    end;
  end;
  with AnImage.Canvas do
  begin
    Brush.Color := Color;
    FillRect(PaintRect);
    Brush.Color := BackColor;
    Pen.Color := BorderColor;
//    Pen.Color := ForeColor;
    Pen.Width := 1;
    Pie(X, Y, W, H * 2 - 1, X + W, PaintRect.Bottom - 1, X, PaintRect.Bottom - 1);
    MoveTo(X, PaintRect.Bottom);
    LineTo(X + W, PaintRect.Bottom);
    if PercentDone > 0 then
    begin
      Pen.Color := ForeColor;
      MiddleX := Width div 2;
      MoveTo(MiddleX, PaintRect.Bottom - 1);
      Angle := (Pi * ((PercentDone / 100)));
      LineTo(Round(MiddleX * (1 - Cos(Angle))), Round((PaintRect.Bottom - 1) *
        (1 - Sin(Angle))));
    end;
  end;
end;

procedure TTobyGauge.SeTTobyGaugeKind(Value: TTobyGaugeKind);
begin
  if Value <> FKind then
  begin
    FKind := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetShowText(Value: Boolean);
begin
  if Value <> FShowText then
  begin
    FShowText := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetInvPie(Value: Boolean);
begin
  if Value <> FInvPie then
  begin
    FInvPie:= Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetCenter(Value: Boolean);
begin
  if Value <> FCenter then
  begin
    FCenter:= Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetBorderStyle(Value: TBorderStyle);
begin
  if Value <> FBorderStyle then
  begin
    FBorderStyle := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetForeColor(Value: TColor);
begin
  if Value <> FForeColor then
  begin
    FForeColor := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetBackColor(Value: TColor);
begin
  if Value <> FBackColor then
  begin
    FBackColor := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetBorderColor(Value: TColor);
begin
  if Value <> FBorderColor then
  begin
    FBorderColor := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetTextXOffs(Value: Longint);
begin
 if Value <> FTextXOffs then
  begin
    FTextXOffs:= Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetTextYOffs(Value: Longint);
begin
 if Value <> FTextYOffs then
  begin
    FTextYOffs:= Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetMinValue(Value: Longint);
begin
  if Value <> FMinValue then
  begin
    if Value > FMaxValue then
      if not (csLoading in ComponentState) then
        raise EInvalidOperation.CreateFmt(SOutOfRange, [-MaxInt, FMaxValue - 1]);
    FMinValue := Value;
    if FCurValue < Value then FCurValue := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetMaxValue(Value: Longint);
begin
  if Value <> FMaxValue then
  begin
    if Value < FMinValue then
      if not (csLoading in ComponentState) then
        raise EInvalidOperation.CreateFmt(SOutOfRange, [FMinValue + 1, MaxInt]);
    FMaxValue := Value;
    if FCurValue > Value then FCurValue := Value;
    Refresh;
  end;
end;

procedure TTobyGauge.SetProgress(Value: Longint);
var
  TempPercent: Longint;
begin
  TempPercent := GetPercentDone;  { remember where we were }
  if Value < FMinValue then
    Value := FMinValue
  else if Value > FMaxValue then
    Value := FMaxValue;
  if FCurValue <> Value then
  begin
    FCurValue := Value;
    if TempPercent <> GetPercentDone then { only refresh if percentage changed }
      Refresh;
  end;
end;

procedure Register;
begin
  RegisterComponents('Tobybear', [TTobyGauge]);
end;

procedure TTobyGauge.AddProgress(Value: Longint);
begin
  Progress := FCurValue + Value;
  Refresh;
end;

end.
