// Created in 2016-2019 by Ryan A. Colyer.
// This work is released with CC0 into the public domain.
// https://creativecommons.org/publicdomain/zero/1.0/


module SmoothCylinder(radius, height, smooth_rad) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  if (radius > smooth_rad) {
    rotate_extrude(convexity=10)
      hull() {
        square([radius-smooth_rad, height]);
        intersection() {
          translate([0, -0.1*height])
            square([1.1*radius, 1.2*height]);
          union() {
            translate([radius-smooth_rad, smooth_rad])
              circle(r=smooth_rad);
            translate([radius-smooth_rad, height-smooth_rad])
              circle(r=smooth_rad);
          }
        }
      }
  }
  else {
    rotate_extrude(convexity=10)
      hull() {
        intersection() {
          translate([0, -0.1*height])
            square([1.1*radius, 1.2*height]);
          union() {
            translate([0, radius])
              circle(r=radius);
            translate([0, height-radius])
              circle(r=radius);
          }
        }
      }
  }
}


module HollowCylinder(outer_rad, inner_rad, height) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  rad_diff = outer_rad-inner_rad;
  rotate_extrude(convexity=10)
    translate([(outer_rad+inner_rad)/2, 0, 0])
    hull() {
      translate([0, rad_diff/2, 0])
        circle(r=rad_diff/2);
      translate([0, height-rad_diff/2, 0])
        circle(r=rad_diff/2);
    }
}

module SmoothHole(radius, height, smooth_rad,
  position=[0,0,0], rotation=[0,0,0]) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  extra_height = 0.002 * height;
  
  difference() {
    children();
    translate(position)
      rotate(rotation)
      translate([0, 0, -extra_height/2])
      difference() {
        translate([0, 0, -extra_height])
          cylinder(r=radius+smooth_rad, h=height+2*extra_height);
        HollowCylinder(radius+2*smooth_rad, radius, height+extra_height);
      }
  }
}


module ChamferHole(radius, height, chamfer_size,
  position=[0,0,0], rotation=[0,0,0]) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  cham = chamfer_size < height/2 - 0.001 ? chamfer_size : height/2 - 0.001;
  extra_height = 0.002 * height;
  height_ex = height+extra_height;
  
  difference() {
    children();
    translate(position)
      rotate(rotation)
      translate([0, 0, -extra_height/2])
      rotate_extrude()
        polygon([[0,0], [0,height_ex], [radius+cham,height_ex],
          [radius,height_ex-cham], [radius,cham],
          [radius+cham,0]]);
  }
}


module SmoothCube(size, smooth_rad) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  size = is_num(size) ? [size, size, size] : size;
  smooth_rad = is_num(smooth_rad) ? [smooth_rad, smooth_rad, smooth_rad] :
    smooth_rad;
  smooth_base = smooth_rad[0];
  scales = smooth_rad / smooth_base;

  scalex = scales[0] * ((smooth_rad[0] < size[0]/2) ? 1 : size[0]/(2*smooth_rad[0]));
  scaley = scales[1] * ((smooth_rad[1] < size[1]/2) ? 1 : size[1]/(2*smooth_rad[1]));
  scalez = scales[2] * ((smooth_rad[2] < size[2]/2) ? 1 : size[2]/(2*smooth_rad[2]));
  smoothx = smooth_rad[0] * scalex / scales[0];
  smoothy = smooth_rad[1] * scaley / scales[1];
  smoothz = smooth_rad[2] * scalez / scales[2];

  hull() {
    translate([smoothx, smoothy, smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([size[0]-smoothx, smoothy, smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([smoothx, size[1]-smoothy, smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([smoothx, smoothy, size[2]-smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([size[0]-smoothx, size[1]-smoothy, smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([size[0]-smoothx, smoothy, size[2]-smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([smoothx, size[1]-smoothy, size[2]-smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
    translate([size[0]-smoothx, size[1]-smoothy, size[2]-smoothz])
      scale([scalex, scaley, scalez])
      sphere(r=smooth_base);
  }
}


module SmoothXYCube(size, smooth_rad) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  size = is_num(size) ? [size, size, size] : size;

  scalex = (smooth_rad < size[0]/2) ? 1 : size[0]/(2*smooth_rad);
  scaley = (smooth_rad < size[1]/2) ? 1 : size[1]/(2*smooth_rad);
  smoothx = smooth_rad * scalex;
  smoothy = smooth_rad * scaley;

  linear_extrude(size[2]) hull() {
    translate([smoothx, smoothy])
      scale([scalex, scaley])
      circle(r=smooth_rad);
    translate([size[0]-smoothx, smoothy])
      scale([scalex, scaley])
      circle(r=smooth_rad);
    translate([smoothx, size[1]-smoothy])
      scale([scalex, scaley])
      circle(r=smooth_rad);
    translate([size[0]-smoothx, size[1]-smoothy])
      scale([scalex, scaley])
      circle(r=smooth_rad);
  }
}


module SmoothCorner(height, width, inner_curv=0) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  translate([width+inner_curv, width+inner_curv, 0])
    rotate([0, 0, 180])
    rotate_extrude(angle=90, convexity=10)
    translate([width/2+inner_curv, 0, 0])
    hull() {
      translate([0, width/2, 0])
        circle(r=width/2);
      translate([0, height-width/2, 0])
        circle(r=width/2);
    }
}


module SmoothWall(width, height, wall_width) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  rotate([-90, 180, -90])
  linear_extrude(width)
    translate([wall_width/2, 0, 0])
    hull() {
      translate([0, wall_width/2, 0])
        circle(r=wall_width/2);
      translate([0, height-wall_width/2, 0])
        circle(r=wall_width/2);
    }
}


module SmoothHollowCube(size, wall_width, inner_curv=0) {
  $fa = ($fa >= 12) ? 1 : $fa;
  $fs = ($fs >= 2) ? 0.4 : $fs;

  size = is_num(size) ? [size, size, size] : size;

  wall_width = wall_width < min(size)/2 ? wall_width : min(size)/2;
  max_inner_curv = (min(size)-2*wall_width)/2;
  inner_curv = inner_curv < max_inner_curv ? inner_curv : max_inner_curv;

  SmoothCorner(size[2], wall_width, inner_curv);
  translate([size[0], 0, 0])
    rotate([0, 0, 90])
    SmoothCorner(size[2], wall_width, inner_curv);
  translate([size[0], size[1], 0])
    rotate([0, 0, 180])
    SmoothCorner(size[2], wall_width, inner_curv);
  translate([0, size[1], 0])
    rotate([0, 0, 270])
    SmoothCorner(size[2], wall_width, inner_curv);

  wall_lengthx = size[0]-2*wall_width-2*inner_curv+0.002;
  wall_lengthy = size[1]-2*wall_width-2*inner_curv+0.002;
  wall_height = size[2];

  translate([size[0], wall_width+inner_curv-0.001, 0])
    rotate([0, 0, 90])
    SmoothWall(wall_lengthy, wall_height, wall_width);
  translate([wall_width, wall_width+inner_curv-0.001, 0])
    rotate([0, 0, 90])
    SmoothWall(wall_lengthy, wall_height, wall_width);
  translate([wall_width+inner_curv-0.001, 0, 0])
    SmoothWall(wall_lengthx, wall_height, wall_width);
  translate([wall_width+inner_curv-0.001, size[1]-wall_width, 0])
    SmoothWall(wall_lengthx, wall_height, wall_width);
}


module CutCorner(insetby, cornerpos=[0,0,0], positives=[undef,undef,undef]) {
  clearance = 4*insetby + 0.001;
  positives = positives==[undef,undef,undef] ? cornerpos : positives;
  angle_offset = (positives[0]?90:0) + (positives[1]?-90:0)
    + ((positives[0]&&positives[1])?180:0);

  difference() {
    children();
    translate(cornerpos)
    rotate([0, 0, 225+angle_offset])
    rotate([0, positives[2]?-45:45, 0])
    translate([-insetby, -clearance/2, -clearance/2])
    cube([clearance, clearance, clearance]);
  }
}

module CutCube(size, insetby=0) {
  size = is_num(size) ? [size, size, size] : size;

  CutCorner(insetby, [0,0,0])
  CutCorner(insetby, [0,0,size[2]])
  CutCorner(insetby, [0,size[1],0])
  CutCorner(insetby, [0,size[1],size[2]])
  CutCorner(insetby, [size[0],0,0])
  CutCorner(insetby, [size[0],0,size[2]])
  CutCorner(insetby, [size[0],size[1],0])
  CutCorner(insetby, [size[0],size[1],size[2]])
  cube(size);
}



module Demo() {
  translate([0, -50, 0])
    HollowCylinder(22, 20, 10);

  SmoothHole(5, 10, 2)
    SmoothCylinder(22, 10, 2);

  translate([30, -30, 0]) SmoothCube([10, 14, 18], 2);

  translate([50, 0, 0]) CutCube([15, 14, 18], 3);

  translate([30, 0, 0]) SmoothXYCube([10, 14, 18], 2);

  translate([57, -23, 0]) SmoothCylinder(5, 18, 2);

  translate([35, -60, 0]) SmoothHollowCube([22, 20, 14], 3, 2);

  translate([80, 0, 0])
    ChamferHole(4, 10, 2, [10, 10, 0])
      cube([20, 20, 10]);
}


Demo();


