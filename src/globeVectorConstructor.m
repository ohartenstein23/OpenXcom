function [lat lon] = globeVectorConstructor(x, y, z)
  
  norm = sqrt(x.^2 + y.^2 + z.^2);
  x = x/norm;
  y = y/norm;
  z = z/norm;
  
  lat = rad2deg(asin(z));
  lat = min(max(-90, lat), 90);
  if (y > 0)
    lon = rad2deg(atan2(y, x));
  else
    lon = 360 + rad2deg(atan(y, x));
  endif
  
  #lon = mod(lon, 360);
  
endfunction