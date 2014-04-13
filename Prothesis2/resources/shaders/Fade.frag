uniform sampler2D txt;
uniform vec3 fadeColor;
uniform float fadeValue;

void main()
{
	vec2 uv = gl_TexCoord[ 0 ].st;
	vec3 color = texture2D( txt, uv ).rgb;
	gl_FragColor = vec4( mix( color, fadeColor, fadeValue ), 1. );
}

