	#version 440 core
	layout (location = 0) in vec3 position; 
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uv;
	layout (location = 3) in vec4 boneIds;
	layout (location = 4) in vec4 boneWeights;
	layout (location = 5) in uint a_texture_id;

	out vec2 TextureCoords;
	out vec3 v_normal;
	out vec3 v_pos;
	out vec4 bw;

	out flat uint TextureID;

	uniform mat4 bone_transforms[200];
	uniform mat4 u_ProjectionView;
	uniform mat4 u_Transform;

	void main()
	{
		bw = vec4(0);
		if(int(boneIds.x) == 1)
		bw.z = boneIds.x;
		vec4 weights = normalize(boneWeights);
		mat4 boneTransform  =    bone_transforms[int(boneIds.x)] * weights.x;
		boneTransform  +=    bone_transforms[int(boneIds.y)] * weights.y;	
		boneTransform  +=    bone_transforms[int(boneIds.z)] * weights.z;
		boneTransform  +=    bone_transforms[int(boneIds.w)] * weights.w;
		vec4 pos = boneTransform * vec4(position, 1.0);
		gl_Position = u_ProjectionView * u_Transform * pos;
		v_pos = vec3(u_Transform * boneTransform * pos);
		TextureCoords = uv;
		v_normal = mat3(transpose(inverse(u_Transform * boneTransform))) * normal;
		v_normal = normalize(v_normal);

		TextureID = a_texture_id;
	}
