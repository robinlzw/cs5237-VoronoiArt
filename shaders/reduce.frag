#extension GL_ARB_texture_rectangle : require

uniform sampler2DRect InputTex;
uniform float InputCols;
uniform float InputRows;
uniform float ReduceFact;
uniform int PassCount;

uniform int OffsetX;
uniform int OffsetY;

uniform sampler2DRect PolyMaskTex;
uniform sampler2DRect ImageTex;


void main()
{
    //*
	if (PassCount == 0) {
		// Copy the texture from the ImageTex
        // Because of the way this shader is drawn, we add offset x&y..
        
        // Texture color
        vec3 col = vec3(texture2DRect(ImageTex, gl_FragCoord.xy + vec2(OffsetX, OffsetY)));
        
        // Mask value
        float b = texture2DRect(PolyMaskTex, gl_FragCoord.xy + vec2(OffsetX, OffsetY)).r;
        
        if (b > 0) {
            gl_FragColor = vec4(col, 1);
        } else {
            gl_FragColor = vec4(0);
        }
	} else {
		vec4 sum = vec4(0);
		vec2 inputTexPos = ReduceFact * (gl_FragCoord.xy - vec2(0.5));
		
		vec2 completeOutputColsRows = floor(vec2(InputCols, InputRows) / ReduceFact);
		vec2 extraInputColsRows = vec2(InputCols, InputRows) - completeOutputColsRows * ReduceFact;
		
		vec2 range;
		range.x = ( gl_FragCoord.x < completeOutputColsRows.x ) ? ReduceFact : extraInputColsRows.x;
		range.y = ( gl_FragCoord.y < completeOutputColsRows.y ) ? ReduceFact : extraInputColsRows.y;
		
        int numNonZero = 0;
        
		for (float dx = 0.5; dx < range.x; dx += 1.0) {
			for (float dy = 0.5; dy < range.y; dy += 1.0) {
                vec4 tmpCol = texture2DRect(InputTex, inputTexPos + vec2(dx, dy));
                
                if (tmpCol.a > 0.0f) {
                    numNonZero += 1;
                }
            
                sum += tmpCol;
            }
        }
		
        float a = numNonZero > 0 ? 1.0f : 0.0f;
        numNonZero = numNonZero > 0 ? numNonZero : 1;
        
		gl_FragColor = vec4(vec3(sum) / float(numNonZero), a);
        //gl_FragColor = vec4(80.0, 170.0, 224.0, 1.0);
	}
    // */
    
    //gl_FragColor = vec4(123.0f, 1.0f, 1.0f, 1.0f); //vec4(col, b);
    //gl_FragColor = vec4(80.0, 170.0, 224.0, 1.0);
}
