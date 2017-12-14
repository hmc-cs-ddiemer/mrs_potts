/*
  Bailey vertex shader: sets the vertex and calculate the color value for pixel and pass it to fragment shader.
    New feature: show the object to be taller according to command line input
                 makes things shinier using keyboard input

*/
varying vec3 color;
uniform float BeatTimeDiff;
uniform float PitchTimeDiff;
uniform float Pitch;
uniform float Octave;
uniform float PastPitch;
uniform float Volume;

vec3 get_pitch_color(float pitch) {

    if (pitch == 1.0) {
        return vec3(.2) * Octave + vec3(0.3686274509803922, 0.7411764705882353, 0.24313725490196078);
    }
    else if (pitch == 2.0) {
        return vec3(.2) * Octave + (vec3(0.3686274509803922, 0.7411764705882353, 0.24313725490196078) + vec3(1.0, 0.7254901960784313, 0.0)) * 0.5;
    }
    else if (pitch == 3.0) {
        return vec3(.2) * Octave + vec3(1.0, 0.7254901960784313, 0.0);
    }
    else if (pitch == 3.0) {
        return vec3(.2) * Octave + vec3(0.9686274509803922, 0.5098039215686274, 0.0);
    }
    else if (pitch == 4.0) {
        return vec3(.2) * Octave + (vec3(0.9686274509803922, 0.5098039215686274, 0.0) + vec3(0.8862745098039215, 0.2196078431372549, 0.2196078431372549)) * 0.5;
    }
    else if (pitch == 5.0) {
        return vec3(.2) * Octave + vec3(0.8862745098039215, 0.2196078431372549, 0.2196078431372549);
    }
    else if (pitch == 6.0) {
        return vec3(.2) * Octave + (vec3(0.8862745098039215, 0.2196078431372549, 0.2196078431372549) + vec3(0.592156862745098, 0.2235294117647059, 0.6)) * 0.5;
    }
    else if (pitch == 7.0) {
        return vec3(.2) * Octave + vec3(0.592156862745098, 0.2235294117647059, 0.6);
    }
    else if (pitch == 8.0) {
        return vec3(.2) * Octave + vec3(0.0, 0.611764705882353, 0.8745098039215686);
    }
    else if (pitch == 9.0) {
        return vec3(.2) * Octave + (vec3(0.0, 0.611764705882353, 0.8745098039215686) + vec3(0.0, 0.611764705882353, 0.8745098039215686)) * 0.5;
    }
    else if (pitch == 10.0) {
        return vec3(.2) * Octave + vec3(0.0, 0.611764705882353, 0.8745098039215686);
    }
    else {
        return vec3(.2) * Octave + (vec3(0.0, 0.611764705882353, 0.8745098039215686) + vec3(0.3686274509803922, 0.7411764705882353, 0.24313725490196078)) * 0.5;
    }
}

void main()
{
    vec3 brighter = vec3(.0);
    // we want to render the shape using standard OpenGL position transforms.
    gl_Position = ftransform();

    // calculate the light position and reflection and normalize it 
    // using the default gl_LightSource and position of the vertex.
    vec3 vertex_normal = gl_Normal;
    vec3 vertex_light_position = normalize(gl_LightSource[0].position.xyz);
    vec3 vertex_light_reflection = normalize(vertex_light_position 
        - 2.0*(dot(vertex_light_position, vertex_normal)*vertex_normal));

    float scale = 1.0;
    // if on beat, pulse!
    if (BeatTimeDiff < 0.1) {
        scale = 1.4 - BeatTimeDiff * 4.0;
    } 

    // scale if necesary
    vec4 pos = gl_Vertex;   
    pos.x = pos.x * scale;
    pos.y = pos.y * scale;   
    pos.z = pos.z * scale;     
    gl_Position = gl_ModelViewProjectionMatrix * pos; 

    vec3 pitch_color = get_pitch_color(Pitch);
    vec3 pastPitch_color = get_pitch_color(PastPitch);
    vec3 object_color = pitch_color;

    // depending a Pitch (between 0 to 1), make teapot different color
    if (PitchTimeDiff < 0.05) {
        object_color =  pastPitch_color *(1.0 - PitchTimeDiff*20.0) + 
            pitch_color * PitchTimeDiff*20.0;
    } 

    // make sin wave indicate volume
    if (pos.z > cos(pos.x/1.2) + Volume*3.0) {
        object_color = gl_Color.xyz;
    }

    // view vector: vector between camera and the vertax that we are
    // looking at currently.
    vec3 view_vector = normalize(vec3(gl_Position.xyz - gl_Vertex.xyz));

    vec3 ambient_color = object_color.xyz * gl_LightSource[0].ambient.xyz;

    vec3 diffuse_color = object_color.xyz * gl_LightSource[0].diffuse.xyz;

    float diffuse_value = max(dot(vertex_normal, vertex_light_position), 0.0);

    vec3 specular_color = vec3(1., 1., 1.) 
        * gl_LightSource[0].specular.xyz 
        * pow(
            max(dot(view_vector, vertex_light_reflection), 0.0), 
            gl_FrontMaterial.shininess
        );

    color = ambient_color 
        + clamp(diffuse_color * diffuse_value, 0., 1.) 
        + specular_color;
}
