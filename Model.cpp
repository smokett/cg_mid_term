#include "Model.h";

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

Model::Model(std::string file_name,GLuint shaderID) {
	model_data = load_mesh(file_name.c_str());
	shaderProgramID = shaderID;
	generateObjectBufferMesh();

}

ModelData Model::load_mesh(const char* file_name) {
	ModelData modelDatap;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelDatap;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		vertex_nums.push_back(mesh->mNumVertices);
		modelDatap.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelDatap.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelDatap.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelDatap.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}
	aiReleaseImport(scene);
	return modelDatap;
}

#pragma endregion MESH LOADING

#pragma region VBO_FUNCTIONS
void Model::generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	//unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	//glGenBuffers(1, &vp_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	//unsigned int vn_vbo = 0;
	//glGenBuffers(1, &vn_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	////	This is for texture coordinates which you don't currently need, so I have commented it out
	////	unsigned int vt_vbo = 0;
	////	glGenBuffers (1, &vt_vbo);
	////	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	////	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);

	//unsigned int vao = 0;
	//glBindVertexArray(vao);

	//glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	////	This is for texture coordinates which you don't currently need, so I have commented it out
	////	glEnableVertexAttribArray (loc3);
	////	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	////	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	int current_count = 0;
	for (unsigned int i = 0; i < vertex_nums.size(); i++) {
		GLuint vp;
		glGenBuffers(1, &vp);
		glBindBuffer(GL_ARRAY_BUFFER, vp);
		glBufferData(GL_ARRAY_BUFFER, vertex_nums[i] * sizeof(vec3), &model_data.mVertices[0] + current_count, GL_STATIC_DRAW);
		current_count += vertex_nums[i];
		vp_vbo.push_back(vp);
	}
	current_count = 0;
	for (unsigned int i = 0; i < vertex_nums.size(); i++) {
		GLuint vn;
		glGenBuffers(1, &vn);
		glBindBuffer(GL_ARRAY_BUFFER, vn);
		glBufferData(GL_ARRAY_BUFFER, vertex_nums[i] * sizeof(vec3), &model_data.mNormals[0] + current_count, GL_STATIC_DRAW);
		current_count += vertex_nums[i];
		vn_vbo.push_back(vn);
	}
	vao = 0;
	glBindVertexArray(vao);
	glEnableVertexAttribArray(loc1);
	glEnableVertexAttribArray(loc2);

}

#pragma endregion VBO_FUNCTIONS