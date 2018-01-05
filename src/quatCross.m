function quat = quatCross(quat1, quat2);
  quat = [quat1(1) * quat2(1) - quat1(2) * quat2(2) - quat1(3) * quat2(3) - quat1(4) * quat2(4),
  quat1(1) * quat2(2) + quat1(2) * quat2(1) - quat1(3) * quat2(4) + quat1(4) * quat2(3),
  quat1(1) * quat2(3) + quat1(2) * quat2(4) + quat1(3) * quat2(1) - quat1(4) * quat2(2),
  quat1(1) * quat2(4) - quat1(2) * quat2(3) + quat1(3) * quat2(2) + quat1(4) * quat2(1)];
endfunction