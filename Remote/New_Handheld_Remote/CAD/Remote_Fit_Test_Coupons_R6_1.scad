// Flytec Remote R6.1 fit-check coupons
// Print these small coupons before attempting the full enclosure.
// part="buttons" | "joystick" | "usb_screws" | "release_guard" | "all"
part="all"; $fn=72;
plate_t=4; label_h=0.55; text_size=3.0;
module label(txt,x,y){translate([x,y,plate_t]) linear_extrude(label_h) text(txt,size=text_size,halign="center",valign="center");}
module button_coupon(){
 difference(){cube([82,36,plate_t],center=true);for(i=[0:3])translate([(i-1.5)*18,0,0])cylinder(d=[13.8,14.2,14.6,15.0][i],h=plate_t+1,center=true);}
 for(i=[0:3])label(str([13.8,14.2,14.6,15.0][i]),(i-1.5)*18,12);label("BUTTON FIT",0,-12);
}
module joystick_coupon(){
 difference(){cube([84,62,plate_t],center=true);for(i=[0:3])translate([(i-1.5)*20,0,0])cylinder(d=[24.5,25.0,25.5,26.0][i],h=plate_t+1,center=true);}
 for(i=[0:3])label(str([24.5,25.0,25.5,26.0][i]),(i-1.5)*20,15);label("STICK APERTURE",0,-17);
}
module usb_screw_coupon(){
 difference(){cube([92,62,plate_t],center=true);for(i=[0:2],j=[0:2])translate([(i-1)*27,(j-1)*17,0])cube([[15.0,15.5,16.0][i],[8.0,8.5,9.0][j],plate_t+1],center=true);for(i=[0:3])translate([-35+i*23,-26,0])cylinder(d=[2.8,3.0,3.2,3.4][i],h=plate_t+1,center=true);}
 for(i=[0:2])label(str([15.0,15.5,16.0][i]),(i-1)*27,26);for(j=[0:2])label(str([8.0,8.5,9.0][j]),-42,(j-1)*17);label("USB SLOT / SCREWS",8,-24);
}
module release_coupon(){
 difference(){cube([62,45,plate_t],center=true);translate([-14,0,0])cylinder(d=17.6,h=plate_t+1,center=true);translate([14,0,0])cylinder(d=18.0,h=plate_t+1,center=true);}
 translate([-14,0,plate_t])difference(){cylinder(d=21,h=2);translate([0,0,-.1])cylinder(d=17.6,h=2.2);}
 translate([14,0,plate_t])difference(){cylinder(d=21,h=2);translate([0,0,-.1])cylinder(d=18.0,h=2.2);}
 label("17.6",-14,14);label("18.0",14,14);label("GUARD / CAP GAP",0,-15);
}
module all_coupon(){translate([-60,45,0])button_coupon();translate([60,45,0])joystick_coupon();translate([-60,-45,0])usb_screw_coupon();translate([60,-45,0])release_coupon();}
if(part=="buttons")button_coupon();else if(part=="joystick")joystick_coupon();else if(part=="usb_screws")usb_screw_coupon();else if(part=="release_guard")release_coupon();else all_coupon();
