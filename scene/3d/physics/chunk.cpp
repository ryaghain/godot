#include "chunk.h"

void Chunk::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
		} break;

		case NOTIFICATION_PARENTED: {
		} break;
	}
}

void Chunk::set_chunk_size(const uint64_t &p_chunk_size) {chunk_data.size = p_chunk_size;}
uint64_t Chunk::get_chunk_size() const {return chunk_data.size;}

void Chunk::set_chunk_position(const Vector3i &p_chunk_position) {
	chunk_data.position = p_chunk_position;
	set_position(Vector3(chunk_data.position * chunk_data.size));
}
Vector3i Chunk::get_chunk_position() const {return chunk_data.position;}

void Chunk::set_chunk_mesh(Ref<ChunkMesh> &p_chunk_mesh) {
	chunk_data.chunk_mesh = p_chunk_mesh;
};
Ref<ChunkMesh> Chunk::get_chunk_mesh() const {return chunk_data.chunk_mesh;}

void Chunk::set_chunk_shape(Ref<ChunkShape> &p_chunk_shape) {
	chunk_data.chunk_shape = p_chunk_shape;
};
Ref<ChunkShape> Chunk::get_chunk_shape() const {return chunk_data.chunk_shape;}

void Chunk::set_chunk_outline(Ref<ChunkMesh> &p_chunk_outline) {
	chunk_data.chunk_outline = p_chunk_outline;
};
Ref<ChunkMesh> Chunk::get_chunk_outline() const {return chunk_data.chunk_outline;}

void Chunk::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_chunk_size", "chunk_position"), &Chunk::set_chunk_size);
    ClassDB::bind_method(D_METHOD("get_chunk_size"), &Chunk::get_chunk_size);

	ClassDB::bind_method(D_METHOD("set_chunk_position", "chunk_position"), &Chunk::set_chunk_position);
    ClassDB::bind_method(D_METHOD("get_chunk_position"), &Chunk::get_chunk_position);

	ClassDB::bind_method(D_METHOD("set_chunk_mesh", "chunk_mesh"), &Chunk::set_chunk_mesh);
    ClassDB::bind_method(D_METHOD("get_chunk_mesh"), &Chunk::get_chunk_mesh);

	ClassDB::bind_method(D_METHOD("set_chunk_shape", "chunk_shape"), &Chunk::set_chunk_shape);
    ClassDB::bind_method(D_METHOD("get_chunk_shape"), &Chunk::get_chunk_shape);

	ClassDB::bind_method(D_METHOD("set_chunk_outline", "chunk_outline"), &Chunk::set_chunk_outline);
    ClassDB::bind_method(D_METHOD("get_chunk_outline"), &Chunk::get_chunk_outline);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RESOURCE_TYPE, "Int", PROPERTY_USAGE_EDITOR), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "chunk_position", PROPERTY_HINT_RESOURCE_TYPE, "Vector3i", PROPERTY_USAGE_EDITOR), "set_chunk_position", "get_chunk_position");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chunk_mesh", PROPERTY_HINT_RESOURCE_TYPE, "ChunkMesh", PROPERTY_USAGE_EDITOR), "set_chunk_mesh", "get_chunk_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chunk_shape", PROPERTY_HINT_RESOURCE_TYPE, "ChunkShape", PROPERTY_USAGE_EDITOR), "set_chunk_shape", "get_chunk_shape");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chunk_outline", PROPERTY_HINT_RESOURCE_TYPE, "ChunkMesh", PROPERTY_USAGE_EDITOR), "set_chunk_outline", "get_chunk_outline");
}	

Chunk::Chunk(PhysicsServer3D::BodyMode p_mode) : PhysicsBody3D(p_mode) {
}

Chunk::~Chunk() {
}