// Flytec new handheld remote — reference-image-inspired fit template
// Styling baseline: sculpted matte-black controller, no blue LED/backlight accents
// Units: mm. NOT final component-verified CAD.
// Select "front", "rear", or "assembly" in the part variable.
// Validate all openings against the actual purchased components before printing.

part = "assembly"; // "front", "rear", "assembly"

W = 310;
H = 190;
D = 62;
wall = 3;
front_th = 3.2;
corner_r = 18;

// Provisional screen opening. Check actual bezel, PCB, touch area and mounting.
screen_w = 106.8;
screen_h = 67.0;
screen_y = -3;

// Two button banks, each 2 columns x 4 rows.
button_d = 14;
button_pitch_x = 21;
button_pitch_y = 21;
bank_center_x = 112;
bank_center_y = -30;

// Joystick openings (placeholder diameter; verify actual joystick).
stick_x = 105;
stick_y = 58;
stick_open_d = 24;

// Separate central power button.
power_d = 16;
power_y = 82;

// Rear-facing antenna bulkhead holes (placeholder).
antenna_d = 8.5;
antenna_x = 125;
antenna_y = 82;

// Lower-edge USB-C service-port opening (placeholder).
usb_w = 13;
usb_h = 7;

$fn = 64;

module rounded_box_xy(w,h,z,r) {
  linear_extrude(height=z)
    offset(r=r)
      square([w-2*r,h-2*r],center=true);
}

module button_holes() {
  for (side=[-1,1])
    for (col=[-0.5,0.5])
      for (row=[0:3])
        translate([side*bank_center_x + col*button_pitch_x,
                 bank_center_y + (1.5-row)*button_pitch_y, -0.1])
          cylinder(d=button_d,h=front_th+0.4);
}

module front_panel() {
  difference() {
    rounded_box_xy(W,H,front_th,corner_r);
    // Screen aperture
    translate([0,screen_y,front_th/2])
      cube([screen_w,screen_h,front_th+0.4],center=true);
    // Power button aperture
    translate([0,power_y,-0.1])
      cylinder(d=power_d,h=front_th+0.4);
    // Joystick apertures
    for (side=[-1,1])
      translate([side*stick_x,stick_y,-0.1])
        cylinder(d=stick_open_d,h=front_th+0.4);
    button_holes();
  }
}

module rear_shell() {
  difference() {
    // Tray body; open face is at z=D for mating with the front panel.
    rounded_box_xy(W,H,D,corner_r);
    translate([0,0,wall])
      rounded_box_xy(W-2*wall,H-2*wall,D-wall+0.2,corner_r-wall);

    // Antenna exits cut through the rear wall (z-facing), not the side walls.
    for (side=[-1,1])
      translate([side*antenna_x,antenna_y,D-wall-0.1])
        cylinder(d=antenna_d,h=wall+0.4);

    // USB-C service opening through the lower perimeter wall.
    translate([0,-H/2-0.1,D/2])
      cube([usb_w,usb_h,wall+2],center=true);
  }
}

module assembly_preview() {
  rear_shell();
  // Front panel shown at the open face of the tray for visual fit review.
  translate([0,0,D+0.1])
    front_panel();
}

if (part=="front") front_panel();
if (part=="rear") rear_shell();
if (part=="assembly") assembly_preview();
