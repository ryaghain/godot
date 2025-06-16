#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "core/templates/local_vector.h"
#include "scene/3d/visual_instance_3d.h"
class Skin;
class SkinReference;

class ChunkMesh : public GeometryInstance3D {
	GDCLASS(ChunkMesh, GeometryInstance3D);

protected:
	Ref<Mesh> mesh;
	Ref<Skin> skin;
	Ref<Skin> skin_internal;
	Ref<SkinReference> skin_reference;
	NodePath skeleton_path = NodePath("..");

	LocalVector<float> blend_shape_tracks;
	HashMap<StringName, int> blend_shape_properties;
	Vector<Ref<Material>> surface_override_materials;

	void _mesh_changed();
	void _resolve_skeleton_path();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool surface_index_0 = false;

	void _notification(int p_what);
	static void _bind_methods();

	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

public:
	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_skin(const Ref<Skin> &p_skin);
	Ref<Skin> get_skin() const;

	void set_skeleton_path(const NodePath &p_skeleton);
	NodePath get_skeleton_path();

	Ref<SkinReference> get_skin_reference() const;

	int get_blend_shape_count() const;
	int find_blend_shape_by_name(const StringName &p_name);
	float get_blend_shape_value(int p_blend_shape) const;
	void set_blend_shape_value(int p_blend_shape, float p_value);

	int get_surface_override_material_count() const;
	void set_surface_override_material(int p_surface, const Ref<Material> &p_material);
	Ref<Material> get_surface_override_material(int p_surface) const;
	Ref<Material> get_active_material(int p_surface) const;

	ChunkMesh *create_debug_tangents_node();
	void create_debug_tangents();

	virtual AABB get_aabb() const override;
    
	ChunkMesh();
	~ChunkMesh();
};

#endif // CHUNK_MESH_H