function [x y z] = globeVectorFromAngles(lat, lon)
  
  z = sin(deg2rad(lat));
  y = sin(deg2rad(lon))*cos(deg2rad(lat));
  x = cos(deg2rad(lon))*cos(deg2rad(lat));
  
endfunction