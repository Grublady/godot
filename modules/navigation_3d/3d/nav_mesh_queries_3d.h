/**************************************************************************/
/*  nav_mesh_queries_3d.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "../nav_utils_3d.h"

#include "core/templates/a_hash_map.h"

#include "servers/navigation/navigation_path_query_parameters_3d.h"
#include "servers/navigation/navigation_path_query_result_3d.h"
#include "servers/navigation/navigation_utilities.h"

using namespace NavigationUtilities;

class NavMap3D;
struct NavMapIteration3D;

class NavMeshQueries3D {
public:
	struct PathQuerySlot {
		LocalVector<Nav3D::NavigationPoly> path_corridor;
		Heap<Nav3D::NavigationPoly *, Nav3D::NavPolyTravelCostGreaterThan, Nav3D::NavPolyHeapIndexer> traversable_polys;
		bool in_use = false;
		uint32_t slot_index = 0;
		AHashMap<const Nav3D::Polygon *, uint32_t> poly_to_id;
	};

	struct NavMeshPathQueryTask3D {
		enum TaskStatus {
			QUERY_STARTED,
			QUERY_FINISHED,
			QUERY_FAILED,
			CALLBACK_DISPATCHED,
			CALLBACK_FAILED,
		};

		// Parameters.
		Vector3 start_position;
		Vector3 target_position;
		uint32_t navigation_layers;
		BitField<PathMetadataFlags> metadata_flags = PathMetadataFlags::PATH_INCLUDE_ALL;
		PathfindingAlgorithm pathfinding_algorithm = PathfindingAlgorithm::PATHFINDING_ALGORITHM_ASTAR;
		PathPostProcessing path_postprocessing = PathPostProcessing::PATH_POSTPROCESSING_CORRIDORFUNNEL;
		bool simplify_path = false;
		real_t simplify_epsilon = 0.0;
		bool exclude_regions = false;
		bool include_regions = false;
		LocalVector<RID> excluded_regions;
		LocalVector<RID> included_regions;

		// Path building.
		Vector3 begin_position;
		Vector3 end_position;
		const Nav3D::Polygon *begin_polygon = nullptr;
		const Nav3D::Polygon *end_polygon = nullptr;
		uint32_t least_cost_id = 0;

		// Map.
		Vector3 map_up;
		NavMap3D *map = nullptr;
		PathQuerySlot *path_query_slot = nullptr;

		// Path points.
		LocalVector<Vector3> path_points;
		LocalVector<int32_t> path_meta_point_types;
		LocalVector<RID> path_meta_point_rids;
		LocalVector<int64_t> path_meta_point_owners;

		Ref<NavigationPathQueryParameters3D> query_parameters;
		Ref<NavigationPathQueryResult3D> query_result;
		Callable callback;
		NavMeshPathQueryTask3D::TaskStatus status = NavMeshPathQueryTask3D::TaskStatus::QUERY_STARTED;

		void path_clear() {
			path_points.clear();
			path_meta_point_types.clear();
			path_meta_point_rids.clear();
			path_meta_point_owners.clear();
		}

		void path_reverse() {
			path_points.reverse();
			path_meta_point_types.reverse();
			path_meta_point_rids.reverse();
			path_meta_point_owners.reverse();
		}
	};

	static bool emit_callback(const Callable &p_callback);

	static Vector3 polygons_get_random_point(const LocalVector<Nav3D::Polygon> &p_polygons, uint32_t p_navigation_layers, bool p_uniformly);

	static Vector3 polygons_get_closest_point_to_segment(const LocalVector<Nav3D::Polygon> &p_polygons, const Vector3 &p_from, const Vector3 &p_to, const bool p_use_collision);
	static Vector3 polygons_get_closest_point(const LocalVector<Nav3D::Polygon> &p_polygons, const Vector3 &p_point);
	static Vector3 polygons_get_closest_point_normal(const LocalVector<Nav3D::Polygon> &p_polygons, const Vector3 &p_point);
	static Nav3D::ClosestPointQueryResult polygons_get_closest_point_info(const LocalVector<Nav3D::Polygon> &p_polygons, const Vector3 &p_point);
	static RID polygons_get_closest_point_owner(const LocalVector<Nav3D::Polygon> &p_polygons, const Vector3 &p_point);

	static Vector3 map_iteration_get_closest_point_to_segment(const NavMapIteration3D &p_map_iteration, const Vector3 &p_from, const Vector3 &p_to, const bool p_use_collision);
	static Vector3 map_iteration_get_closest_point(const NavMapIteration3D &p_map_iteration, const Vector3 &p_point);
	static Vector3 map_iteration_get_closest_point_normal(const NavMapIteration3D &p_map_iteration, const Vector3 &p_point);
	static RID map_iteration_get_closest_point_owner(const NavMapIteration3D &p_map_iteration, const Vector3 &p_point);
	static Nav3D::ClosestPointQueryResult map_iteration_get_closest_point_info(const NavMapIteration3D &p_map_iteration, const Vector3 &p_point);
	static Vector3 map_iteration_get_random_point(const NavMapIteration3D &p_map_iteration, uint32_t p_navigation_layers, bool p_uniformly);

	static void map_query_path(NavMap3D *map, const Ref<NavigationPathQueryParameters3D> &p_query_parameters, Ref<NavigationPathQueryResult3D> p_query_result, const Callable &p_callback);

	static void query_task_map_iteration_get_path(NavMeshPathQueryTask3D &p_query_task, const NavMapIteration3D &p_map_iteration);
	static void _query_task_push_back_point_with_metadata(NavMeshPathQueryTask3D &p_query_task, const Vector3 &p_point, const Nav3D::Polygon *p_point_polygon);
	static void _query_task_find_start_end_positions(NavMeshPathQueryTask3D &p_query_task, const NavMapIteration3D &p_map_iteration);
	static void _query_task_build_path_corridor(NavMeshPathQueryTask3D &p_query_task, const NavMapIteration3D &p_map_iteration);
	static void _query_task_post_process_corridorfunnel(NavMeshPathQueryTask3D &p_query_task);
	static void _query_task_post_process_edgecentered(NavMeshPathQueryTask3D &p_query_task);
	static void _query_task_post_process_nopostprocessing(NavMeshPathQueryTask3D &p_query_task);
	static void _query_task_clip_path(NavMeshPathQueryTask3D &p_query_task, const Nav3D::NavigationPoly *from_poly, const Vector3 &p_to_point, const Nav3D::NavigationPoly *p_to_poly);
	static void _query_task_simplified_path_points(NavMeshPathQueryTask3D &p_query_task);
	static bool _query_task_is_connection_owner_usable(const NavMeshPathQueryTask3D &p_query_task, const NavBaseIteration3D *p_owner);

	static void _query_task_search_polygon_connections(NavMeshPathQueryTask3D &p_query_task, const Nav3D::Connection &p_connection, uint32_t p_least_cost_id, const Nav3D::NavigationPoly &p_least_cost_poly, real_t p_poly_enter_cost, const Vector3 &p_end_point);

	static void simplify_path_segment(int p_start_inx, int p_end_inx, const LocalVector<Vector3> &p_points, real_t p_epsilon, LocalVector<uint32_t> &r_simplified_path_indices);
	static LocalVector<uint32_t> get_simplified_path_indices(const LocalVector<Vector3> &p_path, real_t p_epsilon);
};
