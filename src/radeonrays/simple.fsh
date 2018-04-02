/**
 * OpenGL 4 - Example 42
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

uniform sampler2D g_Texture;
uniform int u_fxaaOn;

uniform float u_lumaThreshold;
uniform float u_mulReduce;
uniform float u_minReduce;
uniform vec2 u_texelStep;
uniform float u_maxSpan;

varying vec2 Texcoord;

void main()
{
	// Sampling texel
	vec3 rgbM = texture2D(g_Texture, Texcoord).rgb;

	if (u_fxaaOn == 0)
	{
		gl_FragColor = vec4(rgbM, 1.0);

		return;
	}

	// Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates.
	vec3 rgbNW = textureOffset(g_Texture, Texcoord, ivec2(-1, 1)).rgb;
    vec3 rgbNE = textureOffset(g_Texture, Texcoord, ivec2(1, 1)).rgb;
    vec3 rgbSW = textureOffset(g_Texture, Texcoord, ivec2(-1, -1)).rgb;
    vec3 rgbSE = textureOffset(g_Texture, Texcoord, ivec2(1, -1)).rgb;

	// Scale factor to compute colorimetric luminance
	const vec3 toLuma = vec3(0.299, 0.587, 0.114);

	// Convert from RGB to luma.
	float lumaNW = dot(rgbNW, toLuma);
	float lumaNE = dot(rgbNE, toLuma);
	float lumaSW = dot(rgbSW, toLuma);
	float lumaSE = dot(rgbSE, toLuma);
	float lumaM = dot(rgbM, toLuma);

	// Gather minimum and maximum luma.
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	// Only da AA if contrast is lower than a maximum threshold
	if (lumaMax - lumaMin < lumaMax * u_lumaThreshold)
	{
		gl_FragColor = vec4(rgbM, 1.0);

		return;
	}

	// Sampling is done along the gradient.
	vec2 samplingDirection;
	samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    samplingDirection.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    // Sampling step distance depends on the luma: The brighter the sampled texels, the smaller the final sampling step direction.
    // Brighter areas are less blurred/sharper than dark areas.
    float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * u_mulReduce, u_minReduce);

	// Factor for norming the sampling direction plus adding the brightness influence.
	float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

    // Calculate final sampling direction vector by reducing, clamping to a range and finally adapting to the texture size.
    samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-u_maxSpan, -u_maxSpan), vec2(u_maxSpan, u_maxSpan)) * u_texelStep;

	// Inner samples on the tab.
	vec3 rgbSampleNeg = texture2D(g_Texture, Texcoord + samplingDirection * (1.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePos = texture2D(g_Texture, Texcoord + samplingDirection * (2.0/3.0 - 0.5)).rgb;

	vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;

	// Outer samples on the tab.
	vec3 rgbSampleNegOuter = texture2D(g_Texture, Texcoord + samplingDirection * (0.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePosOuter = texture2D(g_Texture, Texcoord + samplingDirection * (3.0/3.0 - 0.5)).rgb;

	vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;

	// Calculate luma for checking against the minimum and maximum value.
	float lumaFourTab = dot(rgbFourTab, toLuma);

	// Are outer samples of the tab beyond the edge ...
	if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
	{
		// ... yes, so use only two samples.
		gl_FragColor = vec4(rgbTwoTab, 1.0);
	}
	else
	{
		// ... no, so use four samples.
		gl_FragColor = vec4(rgbFourTab, 1.0);
	}
}