// Flytec new handheld remote — preliminary printable fit template
// Units: mm. NOT final component-verified CAD.
// Export front and rear separately by changing part selector.
part = "front"; // "front" or "rear"

W = 310;
H = 190;
D = 62;
wall = 3;
front_th = 3.2;
corner_r = 18;

// Central display opening is a provisional cavity/window opening.
// Verify actual display bezel, PCB, touch area and mounting holes.
screen_w = 106.8;
screen_h = 67.0;
screen_y = -3;

// Button grids: each bank is 2 columns x 4 rows (8 buttons).
button_d = 14;
button_pitch_x = 21;
button_pitch_y = 21;
bank_center_x = 112;
bank_center_y = -30;

// Joystick openings positioned above the button grids.
stick_x = 105;
stick_y = 58;
stick_open_d = 24; // placeholder; use actual joystick bezel dimensions

// Centered power button above screen.
power_d = 16;
power_y = 82;

// Antenna exits in rear shell, upper rear corners.
antenna_d = 8.5; // placeholder for chosen SMA bulkhead/connector
antenna_x = 125;
antenna_y = 82;

usb_w = 13;
usb_h = 7;
usb_y = -88; // provisional lower edge service-port position

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
                 bank_center_y + (1.5-row)*button_pitch_y,0])
          cylinder(d=button_d,h=front_th+1);
}

module front_panel() {
  difference() {
    rounded_box_xy(W,H,front_th,corner_r);
    // Display cutout
    translate([0,screen_y, -0.1])
      cube([screen_w,screen_h,front_th+0.4],center=true);
    // Power button cutout
    translate([0,power_y,-0.1])
      cylinder(d=power_d,h=front_th+0.4);
    // Joystick openings
    for (side=[-1,1])
      translate([side*stick_x,stick_y,-0.1])
        cylinder(d=stick_open_d,h=front_th+0.4);
    button_holes();
  }
}

module rear_shell() {
  // Hollow tray with a simple open top; front panel mates over rim.
  difference() {
    rounded_box_xy(W,H,D,corner_r);
    translate([0,0,wall])
      rounded_box_xy(W-2*wall,H-2*wall,D-wall+0.2,corner_r-wall);
    // Antenna bulkhead exits at the upper rear corners.
    for (side=[-1,1])
      translate([side*antenna_x,antenna_y,D/2])
        rotate([90,0,0])
          cylinder(d=antenna_d,h=H+1,center=true);
    // USB service port placeholder on lower edge.
    translate([0,-H/2-0.1,D/2])
      cube([usb_w,usb_h,wall+2],center=true);
  }
}

if (part=="front") front_panel();
if (part=="rear") rear_shell();
