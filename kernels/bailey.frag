/*
  Bailey fragement shader: this just colors everything pixel according to calculated value from vertax shader
*/

varying vec3 color;

void main()
{
    // clamp the color so that it doesn't go over 1.0
    gl_FragColor = vec4(clamp(color, 0., 1.), 1.);
}
