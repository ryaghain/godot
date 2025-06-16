#ifndef CHUNK_SHAPE_H
#define CHUNK_SHAPE_H

#include "scene/3d/node_3d.h"
#include "scene/resources/3d/shape_3d.h"

class CollisionObject3D;
class ChunkShape : public Node3D {
	GDCLASS(ChunkShape, Node3D);

	Ref<Shape3D> shape;

	uint32_t owner_id = 0;
	CollisionObject3D *collision_object = nullptr;

#ifndef DISABLE_DEPRECATED
	void resource_changed(Ref<Resource> res);
#endif
	bool disabled = false;

protected:
	void _update_in_shape_owner(bool p_xform_only = false);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void make_convex_from_siblings();

	void set_shape(const Ref<Shape3D> &p_shape);
	Ref<Shape3D> get_shape() const;

	void set_disabled(bool p_disabled);
	bool is_disabled() const;

	PackedStringArray get_configuration_warnings() const override;

	ChunkShape();
	~ChunkShape();
};

#endif // CHUNK_SHAPE_H