// Flytec V080 Remote Enclosure R5 - parametric prototype CAD
// Units mm. Select part="front", "rear", "buttons", or "assembly".
// Sculpted matte-black handheld, central 4.3in screen, two joysticks,
// twin 2x4 banks; right bank upper four = H-L, H-R, K-L, K-R.
// Actual hardware dimensions are placeholders pending measurement.
part="assembly";$fn=72;
W=310;H=210;D=64.5;wall=3.0;front_th=3.4;
screen_w=108.0;screen_h=70.0;screen_y=2;
stick_x=97;stick_y=73;stick_cut=24.5;
button_d=14.0;button_pitch_x=19.5;button_pitch_y=20.5;bank_x=119;bank_y=-39;
antenna_hole=8.5;antenna_x=61;antenna_y=94;usb_w=14.5;usb_h=8.0;power_d=16.0;power_y=99;
module outline2d(){scale([0.963,1])offset(r=10)polygon(points=[[-142,-54],[-151,-34],[-151,22],[-140,54],[-121,76],[-96,91],[-70,87],[-48,71],[-29,61],[29,61],[48,71],[70,87],[96,91],[121,76],[140,54],[151,22],[151,-34],[142,-54],[128,-73],[112,-91],[89,-98],[68,-92],[49,-76],[34,-66],[-34,-66],[-49,-76],[-68,-92],[-89,-98],[-112,-91],[-128,-73]]);}
module shell_outline(h){linear_extrude(height=h)outline2d();}
module button_positions(){for(side=[-1,1],col=[-0.5,0.5],row=[0:3])translate([side*bank_x+col*button_pitch_x,bank_y+(1.5-row)*button_pitch_y,0])children();}
module release_positions(){for(col=[-0.5,0.5],row=[0:1])translate([bank_x+col*button_pitch_x,bank_y+(1.5-row)*button_pitch_y,0])children();}
module front_panel(){difference(){shell_outline(front_th);translate([0,screen_y,-.1])linear_extrude(front_th+.2)offset(r=2)square([screen_w-4,screen_h-4],center=true);for(side=[-1,1])translate([side*stick_x,stick_y,-.1])cylinder(d=stick_cut,h=front_th+.2);button_positions()cylinder(d=button_d,h=front_th+.2);translate([0,-91,-.1])cylinder(d=18,h=front_th+.2);translate([0,power_y,-.1])cylinder(d=power_d,h=front_th+.2);translate([0,-H/2-.1,front_th/2])cube([usb_w,usb_h,front_th+1],center=true);for(p=[[-136,45],[136,45],[-136,-50],[136,-50],[-75,-89],[75,-89]])translate([p[0],p[1],-.1])cylinder(d=3.2,h=front_th+.2);}release_positions()translate([0,0,front_th])difference(){cylinder(d=18.2,h=1.5);translate([0,0,-.1])cylinder(d=16.4,h=1.7);}}
module rear_shell(){difference(){shell_outline(D);translate([0,0,wall])shell_outline(D-wall+.2);for(p=[[-136,45],[136,45],[-136,-50],[136,-50],[-75,-89],[75,-89]])translate([p[0],p[1],-.1])cylinder(d=3.2,h=D+.2);for(side=[-1,1])translate([side*antenna_x,antenna_y,D-wall-.1])cylinder(d=antenna_hole,h=wall+.2);translate([0,-H/2-.1,D/2])cube([usb_w,usb_h,wall+2],center=true);for(i=[-2:2])translate([i*8,-78,D-wall-.1])cube([3,18,wall+.2],center=true);}for(p=[[-136,45],[136,45],[-136,-50],[136,-50],[-75,-89],[75,-89]])translate([p[0],p[1],wall])difference(){cylinder(d=9,h=10);translate([0,0,-.1])cylinder(d=2.8,h=10.2);}for(p=[[-45,38],[45,38],[-45,-45],[45,-45]])translate([p[0],p[1],wall])difference(){cylinder(d=12,h=5);translate([0,0,-.1])cylinder(d=3,h=5.2);}}
module cap(label="",release=false){union(){cylinder(d=release?13.6:13.2,h=2.4);translate([0,0,2.4])cylinder(d=release?10.8:10.5,h=4.8);}if(label!="")translate([0,0,.25])linear_extrude(height=.35)text(label,size=release?2.3:2.0,halign="center",valign="center",font="Liberation Sans:style=Bold");}
module button_set(){for(i=[0:7])translate([i*18,0,0])cap();translate([0,25,0])cap("H-L",true);translate([18,25,0])cap("H-R",true);translate([36,25,0])cap("K-L",true);translate([54,25,0])cap("K-R",true);}
module assembly_preview(){color("#202326")rear_shell();translate([0,0,D+.1])color("#292C30")front_panel();translate([0,screen_y,D+.1+front_th-.6])color("#101820")cube([screen_w,screen_h,1.2],center=true);for(side=[-1,1])translate([side*stick_x,stick_y,D+.1+front_th])color("#777B80")cylinder(d=21,h=10);button_positions()translate([0,0,D+.1+front_th])rotate([180,0,0])color("#D5D6D4")cap();release_positions()translate([0,0,D+.1+front_th+1.2])rotate([180,0,0])color("#C9483B")cap("",true);translate([0,-91,D+.1+front_th])rotate([180,0,0])color("#E5E5E5")cap("OK");}
if(part=="front")front_panel();else if(part=="rear")rear_shell();else if(part=="buttons")button_set();else assembly_preview();
