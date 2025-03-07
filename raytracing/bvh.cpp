#include "bvh.h"
#include <algorithm>


bool bvh_node::bounding_box(double t0, double t1, aabb& output_box)const
{
	output_box = box;
	return true;
}
bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec)const
{
	if (!box.hit(r,t_min,t_max))
	{
		return false;
	}
	bool hit_left=left->hit(r,t_min,t_max,rec);
	bool hit_right=right->hit(r,t_min,hit_left?rec.t:t_max,rec);
	return hit_left || hit_right;


}

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis)
{
	aabb box_a;
	aabb box_b;
	if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
	{
		std::cerr << "No Bounding box in bvh_node constructor.\n";
	}
	return box_a.min().e[axis] < box_b.min().e[axis];
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 0);
}
bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 1);
}
bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 2);
}

bvh_node::bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, double time0, double time1)
{
	int axis = random_int(0, 2);
	auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;
	size_t object_span = end - start;

	if (object_span==1)
	{
		left = right = objects[start];
	}
	else if (object_span == 2)
	{
		if (comparator(objects[start],objects[start+1]))
		{
			left = objects[start];
			right = objects[start + 1];

		}
		else
		{
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else
	{
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		auto mid = start + object_span / 2;
		left = make_shared<bvh_node>(objects, start, mid, time0, time1);
		right = make_shared<bvh_node>(objects, mid, end, time0, time1);
	}

	aabb box_left, box_right;
	if (!left->bounding_box(time0,time1,box_left)||!right->bounding_box(time0,time1,box_right))
	{
		std::cerr << "No Bounding box in bvh_node constructor.\n";
	}

	box = surrounding_box(box_left, box_right);
}

