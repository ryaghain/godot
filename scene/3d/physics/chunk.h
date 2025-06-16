#ifndef CHUNK_H
#define CHUNK_H

#include "scene/3d/physics/physics_body_3d.h"
#include "scene/3d/physics/chunk_shape.h"
#include "scene/3d/chunk_mesh.h"

class ChunkMesh;
class Chunk : public PhysicsBody3D {
	GDCLASS(Chunk, PhysicsBody3D);

private:
    struct ChunkData {
        mutable uint64_t size = 128;
        mutable Vector3i position = Vector3i(0,0,0);

        Ref<ChunkMesh> chunk_mesh;
        Ref<ChunkShape> chunk_shape;
        Ref<ChunkMesh> chunk_outline;
    } chunk_data;

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    void set_chunk_size(const uint64_t &p_chunk_size);
    uint64_t get_chunk_size() const;

	void set_chunk_position(const Vector3i &p_chunk_position);
	Vector3i get_chunk_position() const;

    void set_chunk_mesh(Ref<ChunkMesh> &p_chunk_mesh);
    Ref<ChunkMesh> get_chunk_mesh() const;

    void set_chunk_shape(Ref<ChunkShape> &p_chunk_shape);
    Ref<ChunkShape> get_chunk_shape() const;

    void set_chunk_outline(Ref<ChunkMesh> &p_chunk_outline);
    Ref<ChunkMesh> get_chunk_outline() const;

    Chunk(PhysicsServer3D::BodyMode p_mode = PhysicsServer3D::BODY_MODE_STATIC);
    ~Chunk();
};

#endif // CHUNK_H