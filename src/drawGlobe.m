fileToRead = fopen("~/OpenXcom/user/xcom1/geoscapeGeneratorOutput.yml");
intersections = greatCircles = {};
currentLine = 0;
currentNode = "";
cellIndex = 0;

while(currentLine != -1)
  currentLine = fgetl(fileToRead);
  dashIndex = regexp(currentLine, '\-');
  if (length(dashIndex) == 0)
     currentNode = currentLine;
     cellIndex = 0;
  elseif (dashIndex(1) == 5)
     cellIndex++;
  elseif (dashIndex(1) == 7)
     if (strcmp(currentNode, "  polygons:"))
       if (length(intersections) < cellIndex)
         intersections{cellIndex} = [];
       endif
       intersections{cellIndex} = [intersections{cellIndex}; str2num(currentLine(dashIndex(1)+2:end))];
     elseif (strcmp(currentNode, "  greatCircles:"))
       if (length(greatCircles) < cellIndex)
         greatCircles{cellIndex} = [];
       endif
       greatCircles{cellIndex} = [greatCircles{cellIndex}; str2num(currentLine(dashIndex(1)+2:end))];
     endif
  endif
endwhile

fclose(fileToRead);

for (m = 1:length(intersections))

  close;
  hold on;
  drawSphere([0 0 0 1]);

  for (n = 1:length(greatCircles))
    drawCircle3d([0 0 0 1 90-greatCircles{n}(4) greatCircles{n}(5)], 'linewidth', 2);
  endfor

  for (n = 1:length(intersections{m})/2)
    [x y z] = globeVectorFromAngles(intersections{m}(2*n-1), intersections{m}(2*n));
    drawPoint3d([x y z], 'markerfacecolor', 'auto', 'markersize', 10);
  endfor
  xlabel('x'); ylabel('y'); zlabel('z');
  hold off;

  pause;

endfor
