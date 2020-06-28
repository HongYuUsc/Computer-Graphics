 vec3 v0, v1, v2, v3, v4, v5, v6, v7,norm;
  v0 = result + alpha * (-N + B);
  v1 = result + alpha * (N + B);
  v2 = result + alpha * (N - B);
  v3 = result + alpha * (-N - B);

  pushnorm(v0, v1, v2);
  pushnorm(v0, v2, v3);


  for (int i = 0; i < numSplines; i++)
  {
	  for (int j = 0; j < splines[i].numControlPoints - 3; j++)
	  {

		  Control[0] = vec4(splines[i].points[j].x, splines[i].points[j + 1].x, splines[i].points[j + 2].x, splines[i].points[j + 3].x);
		  Control[1] = vec4(splines[i].points[j].y, splines[i].points[j + 1].y, splines[i].points[j + 2].y, splines[i].points[j + 3].y);
		  Control[2] = vec4(splines[i].points[j].z, splines[i].points[j + 1].z, splines[i].points[j + 2].z, splines[i].points[j + 3].z);

		  for (float u = 0.001; u <= 1.0; u = u + 0.001)
		  {
			  result = vec4(u * u * u, u * u, u, 1)*Basis*Control;
			  tang = vec4(3 * u * u, 2 * u, 1, 0)*Basis*Control;
			  push(point,result);
			  push(tangent,tang);

			  N = normalize(cross(B, tang));
			  B = normalize(cross(tang, N));
			  
			  v4 = result + alpha * (-N + B);
			  v5 = result + alpha * (N + B);
			  v6 = result + alpha * (N - B);
			  v7 = result + alpha * (-N - B);

			  pushnorm(v4, v5, v1);
			  pushnorm(v4, v1, v0);
			  pushnorm(v0, v4, v7);
			  pushnorm(v0, v7, v3);
			  pushnorm(v7, v6, v2);
			  pushnorm(v7, v2, v3);
			  pushnorm(v1, v5, v6);
			  pushnorm(v1, v6, v2);
			  pushnorm(v4, v5, v6);
			  pushnorm(v4, v6, v7);
			  
			  v0 = v4; v1 = v5; v2 = v6; v3 = v7;
		  }
	  }
  }