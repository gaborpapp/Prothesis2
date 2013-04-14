#version 110

uniform sampler2D tex0;

varying vec3 v;
varying vec3 N;

void main()
{
/*
	vec3 L = gl_LightSource[ 0 ].position.xyz;
	vec3 l = normalize( L );

	vec3 n = normalize( N );
	vec3 E = normalize( eyeVec );
	vec3 R = reflect( -l, n );

	// ambient term
	vec4 ambient = gl_LightSource[ 0 ].ambient *
				   gl_FrontMaterial.ambient;

	// diffuse term
	vec4 diffuse = gl_LightSource[ 0 ].diffuse * 
				   gl_FrontMaterial.diffuse;
	diffuse *= max( dot( n, l ), 0. );

	// specular term
	vec4 specular = gl_LightSource[ 0 ].specular *
					gl_FrontMaterial.specular;
	specular *= pow( max( dot( E, R ), 0. ), gl_FrontMaterial.shininess );
*/
	const vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
	const vec4 diffuse = vec4(0.9, 0.9, 0.9, 1);
	const vec4 specular = vec4(1, 1, 1, 1);
	const float shininess = 50.0;
	/*
	vec4 ambient = gl_LightSource[ 0 ].ambient *
				   gl_FrontMaterial.ambient;
	vec4 diffuse = gl_LightSource[ 0 ].diffuse * 
				   gl_FrontMaterial.diffuse;
	vec4 specular = gl_LightSource[ 0 ].specular *
					gl_FrontMaterial.specular;
	float shininess = gl_FrontMaterial.shininess;
	*/
	
	//vec3 L = normalize(gl_LightSource[0].position.xyz - v);   
	vec3 L = normalize( gl_LightSource[ 0 ].position.xyz );   
	vec3 E = normalize( -v ); 
	vec3 R = normalize( -reflect( L, N ) );  

	// ambient term 
	vec4 Iamb = ambient;    

	// diffuse term
	vec4 Idiff = diffuse; //texture2D( tex0, gl_TexCoord[0].st) * diffuse; 
	Idiff *= max( dot( N, L ), 0. );
	Idiff = clamp( Idiff, 0., 1. );     

	// specular term
	vec4 Ispec = specular;
	Ispec *= pow( max( dot( R, E), 0. ), shininess );
	Ispec = clamp( Ispec, 0.0, 1.0); 

	// final color 
	gl_FragColor = Iamb + Idiff + Ispec;	
}
