#include "chunk_mesh.h"

#include "scene/3d/physics/collision_shape_3d.h"
#include "scene/3d/physics/static_body_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "scene/resources/3d/convex_polygon_shape_3d.h"

bool ChunkMesh::_set(const StringName &p_name, const Variant &p_value) {
	//this is not _too_ bad performance wise, really. it only arrives here if the property was not set anywhere else.
	//add to it that it's probably found on first call to _set anyway.

	if (!get_instance().is_valid()) {
		return false;
	}

	HashMap<StringName, int>::Iterator E = blend_shape_properties.find(p_name);
	if (E) {
		set_blend_shape_value(E->value, p_value);
		return true;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();

		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}

		set_surface_override_material(idx, p_value);
		return true;
	}

	return false;
}

bool ChunkMesh::_get(const StringName &p_name, Variant &r_ret) const {
	if (!get_instance().is_valid()) {
		return false;
	}

	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		r_ret = get_blend_shape_value(E->value);
		return true;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();
		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}
		r_ret = surface_override_materials[idx];
		return true;
	}
	return false;
}

void ChunkMesh::_get_property_list(List<PropertyInfo> *p_list) const {
	List<String> ls;
	for (const KeyValue<StringName, int> &E : blend_shape_properties) {
		ls.push_back(E.key);
	}

	ls.sort();

	for (const String &E : ls) {
		p_list->push_back(PropertyInfo(Variant::FLOAT, E, PROPERTY_HINT_RANGE, "-1,1,0.00001"));
	}

	if (mesh.is_valid()) {
		for (int i = 0; i < mesh->get_surface_count(); i++) {
			p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("%s/%d", PNAME("surface_material_override"), i), PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_DEFAULT));
		}
	}
}

void ChunkMesh::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh == p_mesh) {
		return;
	}

	if (mesh.is_valid()) {
		mesh->disconnect_changed(callable_mp(this, &ChunkMesh::_mesh_changed));
	}

	mesh = p_mesh;

	if (mesh.is_valid()) {
		// If mesh is a PrimitiveMesh, calling get_rid on it can trigger a changed callback
		// so do this before connecting _mesh_changed.
		set_base(mesh->get_rid());
		mesh->connect_changed(callable_mp(this, &ChunkMesh::_mesh_changed));
		_mesh_changed();
	} else {
		blend_shape_tracks.clear();
		blend_shape_properties.clear();
		set_base(RID());
		update_gizmos();
	}

	notify_property_list_changed();
}

Ref<Mesh> ChunkMesh::get_mesh() const {
	return mesh;
}

int ChunkMesh::get_blend_shape_count() const {
	if (mesh.is_null()) {
		return 0;
	}
	return mesh->get_blend_shape_count();
}
int ChunkMesh::find_blend_shape_by_name(const StringName &p_name) {
	if (mesh.is_null()) {
		return -1;
	}
	for (int i = 0; i < mesh->get_blend_shape_count(); i++) {
		if (mesh->get_blend_shape_name(i) == p_name) {
			return i;
		}
	}
	return -1;
}
float ChunkMesh::get_blend_shape_value(int p_blend_shape) const {
	ERR_FAIL_COND_V(mesh.is_null(), 0.0);
	ERR_FAIL_INDEX_V(p_blend_shape, (int)blend_shape_tracks.size(), 0);
	return blend_shape_tracks[p_blend_shape];
}
void ChunkMesh::set_blend_shape_value(int p_blend_shape, float p_value) {
	ERR_FAIL_COND(mesh.is_null());
	ERR_FAIL_INDEX(p_blend_shape, (int)blend_shape_tracks.size());
	blend_shape_tracks[p_blend_shape] = p_value;
	RenderingServer::get_singleton()->instance_set_blend_shape_weight(get_instance(), p_blend_shape, p_value);
}

void ChunkMesh::_resolve_skeleton_path() {
	Ref<SkinReference> new_skin_reference;

	if (!skeleton_path.is_empty()) {
		Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(get_parent());
		if (skeleton) {
			if (skin_internal.is_null()) {
				new_skin_reference = skeleton->register_skin(skeleton->create_skin_from_rest_transforms());
				//a skin was created for us
				skin_internal = new_skin_reference->get_skin();
				notify_property_list_changed();
			} else {
				new_skin_reference = skeleton->register_skin(skin_internal);
			}
		}
	}

	skin_reference = new_skin_reference;

	if (skin_reference.is_valid()) {
		RenderingServer::get_singleton()->instance_attach_skeleton(get_instance(), skin_reference->get_skeleton());
	} else {
		RenderingServer::get_singleton()->instance_attach_skeleton(get_instance(), RID());
	}
}

void ChunkMesh::set_skin(const Ref<Skin> &p_skin) {
	skin_internal = p_skin;
	skin = p_skin;
	if (!is_inside_tree()) {
		return;
	}
	_resolve_skeleton_path();
}

Ref<Skin> ChunkMesh::get_skin() const {
	return skin;
}

Ref<SkinReference> ChunkMesh::get_skin_reference() const {
	return skin_reference;
}

void ChunkMesh::set_skeleton_path(const NodePath &p_skeleton) {
	skeleton_path = p_skeleton;
	if (!is_inside_tree()) {
		return;
	}
	_resolve_skeleton_path();
}

NodePath ChunkMesh::get_skeleton_path() {
	return skeleton_path;
}

AABB ChunkMesh::get_aabb() const {
	if (!mesh.is_null()) {
		return mesh->get_aabb();
	}

	return AABB();
}

void ChunkMesh::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_resolve_skeleton_path();
		} break;
		case NOTIFICATION_TRANSLATION_CHANGED: {
			if (mesh.is_valid()) {
				mesh->notification(NOTIFICATION_TRANSLATION_CHANGED);
			}
		} break;
	}
}

int ChunkMesh::get_surface_override_material_count() const {
	return surface_override_materials.size();
}

void ChunkMesh::set_surface_override_material(int p_surface, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_surface, surface_override_materials.size());

	surface_override_materials.write[p_surface] = p_material;

	if (surface_override_materials[p_surface].is_valid()) {
		RS::get_singleton()->instance_set_surface_override_material(get_instance(), p_surface, surface_override_materials[p_surface]->get_rid());
	} else {
		RS::get_singleton()->instance_set_surface_override_material(get_instance(), p_surface, RID());
	}
}

Ref<Material> ChunkMesh::get_surface_override_material(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surface_override_materials.size(), Ref<Material>());

	return surface_override_materials[p_surface];
}

Ref<Material> ChunkMesh::get_active_material(int p_surface) const {
	Ref<Material> mat_override = get_material_override();
	if (mat_override.is_valid()) {
		return mat_override;
	}

	Ref<Material> surface_material = get_surface_override_material(p_surface);
	if (surface_material.is_valid()) {
		return surface_material;
	}

	Ref<Mesh> m = get_mesh();
	if (m.is_valid()) {
		return m->surface_get_material(p_surface);
	}

	return Ref<Material>();
}

void ChunkMesh::_mesh_changed() {
	ERR_FAIL_COND(mesh.is_null());
	surface_override_materials.resize(mesh->get_surface_count());

	uint32_t initialize_bs_from = blend_shape_tracks.size();
	blend_shape_tracks.resize(mesh->get_blend_shape_count());

	for (uint32_t i = 0; i < blend_shape_tracks.size(); i++) {
		blend_shape_properties["blend_shapes/" + String(mesh->get_blend_shape_name(i))] = i;
		if (i < initialize_bs_from) {
			set_blend_shape_value(i, blend_shape_tracks[i]);
		} else {
			set_blend_shape_value(i, 0);
		}
	}

	int surface_count = mesh->get_surface_count();
	for (int surface_index = 0; surface_index < surface_count; ++surface_index) {
		if (surface_override_materials[surface_index].is_valid()) {
			RS::get_singleton()->instance_set_surface_override_material(get_instance(), surface_index, surface_override_materials[surface_index]->get_rid());
		}
	}

	update_gizmos();
}

ChunkMesh *ChunkMesh::create_debug_tangents_node() {
	Vector<Vector3> lines;
	Vector<Color> colors;

	Ref<Mesh> m = get_mesh();
	if (!m.is_valid()) {
		return nullptr;
	}

	for (int i = 0; i < m->get_surface_count(); i++) {
		Array arrays = m->surface_get_arrays(i);
		ERR_CONTINUE(arrays.size() != Mesh::ARRAY_MAX);

		Vector<Vector3> verts = arrays[Mesh::ARRAY_VERTEX];
		Vector<Vector3> norms = arrays[Mesh::ARRAY_NORMAL];
		if (norms.size() == 0) {
			continue;
		}
		Vector<float> tangents = arrays[Mesh::ARRAY_TANGENT];
		if (tangents.size() == 0) {
			continue;
		}

		for (int j = 0; j < verts.size(); j++) {
			Vector3 v = verts[j];
			Vector3 n = norms[j];
			Vector3 t = Vector3(tangents[j * 4 + 0], tangents[j * 4 + 1], tangents[j * 4 + 2]);
			Vector3 b = (n.cross(t)).normalized() * tangents[j * 4 + 3];

			lines.push_back(v); //normal
			colors.push_back(Color(0, 0, 1)); //color
			lines.push_back(v + n * 0.04); //normal
			colors.push_back(Color(0, 0, 1)); //color

			lines.push_back(v); //tangent
			colors.push_back(Color(1, 0, 0)); //color
			lines.push_back(v + t * 0.04); //tangent
			colors.push_back(Color(1, 0, 0)); //color

			lines.push_back(v); //binormal
			colors.push_back(Color(0, 1, 0)); //color
			lines.push_back(v + b * 0.04); //binormal
			colors.push_back(Color(0, 1, 0)); //color
		}
	}

	if (lines.size()) {
		Ref<StandardMaterial3D> sm;
		sm.instantiate();

		sm->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
		sm->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
		sm->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		sm->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);

		Ref<ArrayMesh> am;
		am.instantiate();
		Array a;
		a.resize(Mesh::ARRAY_MAX);
		a[Mesh::ARRAY_VERTEX] = lines;
		a[Mesh::ARRAY_COLOR] = colors;

		am->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, a);
		am->surface_set_material(0, sm);

		ChunkMesh *mi = memnew(ChunkMesh);
		mi->set_mesh(am);
		mi->set_name("DebugTangents");
		return mi;
	}

	return nullptr;
}

void ChunkMesh::create_debug_tangents() {
	ChunkMesh *mi = create_debug_tangents_node();
	if (!mi) {
		return;
	}

	add_child(mi, true);
	if (is_inside_tree() && this == get_tree()->get_edited_scene_root()) {
		mi->set_owner(this);
	} else {
		mi->set_owner(get_owner());
	}
}

bool ChunkMesh::_property_can_revert(const StringName &p_name) const {
	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		return true;
	}
	return false;
}

bool ChunkMesh::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	HashMap<StringName, int>::ConstIterator E = blend_shape_properties.find(p_name);
	if (E) {
		r_property = 0.0f;
		return true;
	}
	return false;
}

void ChunkMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkMesh::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &ChunkMesh::get_mesh);
	ClassDB::bind_method(D_METHOD("set_skeleton_path", "skeleton_path"), &ChunkMesh::set_skeleton_path);
	ClassDB::bind_method(D_METHOD("get_skeleton_path"), &ChunkMesh::get_skeleton_path);
	ClassDB::bind_method(D_METHOD("set_skin", "skin"), &ChunkMesh::set_skin);
	ClassDB::bind_method(D_METHOD("get_skin"), &ChunkMesh::get_skin);
	ClassDB::bind_method(D_METHOD("get_skin_reference"), &ChunkMesh::get_skin_reference);

	ClassDB::bind_method(D_METHOD("get_surface_override_material_count"), &ChunkMesh::get_surface_override_material_count);
	ClassDB::bind_method(D_METHOD("set_surface_override_material", "surface", "material"), &ChunkMesh::set_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_surface_override_material", "surface"), &ChunkMesh::get_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_active_material", "surface"), &ChunkMesh::get_active_material);

	ClassDB::bind_method(D_METHOD("get_blend_shape_count"), &ChunkMesh::get_blend_shape_count);
	ClassDB::bind_method(D_METHOD("find_blend_shape_by_name", "name"), &ChunkMesh::find_blend_shape_by_name);
	ClassDB::bind_method(D_METHOD("get_blend_shape_value", "blend_shape_idx"), &ChunkMesh::get_blend_shape_value);
	ClassDB::bind_method(D_METHOD("set_blend_shape_value", "blend_shape_idx", "value"), &ChunkMesh::set_blend_shape_value);

	ClassDB::bind_method(D_METHOD("create_debug_tangents"), &ChunkMesh::create_debug_tangents);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_GROUP("Skeleton", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "skin", PROPERTY_HINT_RESOURCE_TYPE, "Skin"), "set_skin", "get_skin");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "skeleton", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Skeleton3D"), "set_skeleton_path", "get_skeleton_path");
	ADD_GROUP("", "");
}

ChunkMesh::ChunkMesh() {
}

ChunkMesh::~ChunkMesh() {
}