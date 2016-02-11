typedef __attribute__(( ext_vector_type(4) )) float float4;
typedef __attribute__(( ext_vector_type(8) )) float float8;

//float2 vec2, vec2_2;
float4 vec4, vec4_2 = 0.42;
//float f;

float4 test2() {
  /* vec2 = vec4.xy;  // shorten */
/*   f = vec2.x;      // extract elt */
/*   vec4 = vec4.yyyy;  // splat */
/*   vec4.zw = vec2;    // insert */
  return vec4 * vec4_2;
} 
