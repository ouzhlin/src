/**
 * A* ??
 * author: zhangpanyi@live.com
 * https://github.com/zhangpanyi/a-star.git
 */

#include "datastruct/JOAStar.h"

#include <cassert>
#include <cstring>
#include <algorithm>

NS_JOFW_BEGIN

const int kStepValue = 10;
const int kObliqueValue = 14;

JOAStar::JOAStar()
	: width_(0)
	, height_(0)
    , query_(nullptr)
    , gValQuery_(nullptr)
    , bAllowCorner(false)
	, step_value_(kStepValue)
	, oblique_value_(kObliqueValue)
{
	
}

JOAStar::~JOAStar()
{
	clear();	
}

int JOAStar::stepValue() const
{
	return step_value_;
}

int JOAStar::obliqueValue() const
{
	return oblique_value_;
}

void JOAStar::setStepValue(int value)
{
	step_value_ = value;
}

void JOAStar::setObliqueValue(int value)
{
	oblique_value_ = value;
}

void JOAStar::clear()
{	
	std::vector<Node *>::iterator itr = maps_.begin();
	while (itr != maps_.end())
	{
		delete *itr;
		++itr;
	}
	maps_.clear();
	query_ = nullptr;
    gValQuery_ = nullptr;
	open_list_.clear();
	width_ = height_ = 0;
}

bool JOAStar::getNodeIndex(Node *node, size_t &index)
{
	index = 0;
	const size_t size = open_list_.size();
	while (index < size)
	{
		if (open_list_[index]->pos == node->pos)
		{
			return true;
		}
		++index;
	}
	return false;
}

void JOAStar::percolateUp(size_t hole)
{
	size_t parent = 0;
	while (hole > 0)
	{
		parent = (hole - 1) / 2;
		if (open_list_[hole]->f() < open_list_[parent]->f())
		{
			std::swap(open_list_[hole], open_list_[parent]);
			hole = parent;
		}
		else
		{
			return;
		}
	}
}

inline uint16_t JOAStar::calculGValue(Node *parent_node, const Vec2 &current_pos)
{
	uint16_t g_value = ((abs(current_pos.y + current_pos.x - parent_node->pos.y - parent_node->pos.x)) == 2 ? oblique_value_ : step_value_);
    //return g_value += gValQuery_? parent_node->g+gValQuery_(current_pos) : parent_node->g;
	return parent_node->g;
}

inline uint16_t JOAStar::calculHValue(const Vec2 &current_pos, const Vec2 &end_pos)
{
	unsigned int h_value = abs(end_pos.y + end_pos.x - current_pos.y - current_pos.x);
	return h_value * step_value_;
}

inline bool JOAStar::hasNoodeInOpenList(const Vec2 &pos, Node *&out)
{
	out = maps_[pos.y * height_ + pos.x];
	return out ? out->state == IN_OPENLIST : false;
}

inline bool JOAStar::hasNodeInCloseList(const Vec2 &pos)
{
	Node *node_ptr = maps_[pos.y * height_ + pos.x];
	return node_ptr ? node_ptr->state == IN_CLOSELIST : false;
}

bool JOAStar::canreach(const Vec2 &pos)
{
	return (pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_) ? query_(pos) : false;
}

bool JOAStar::canreach(const Vec2 &current_pos, const Vec2 &target_pos)
{
	if (target_pos.x >= 0 && target_pos.x < width_ && target_pos.y >= 0 && target_pos.y < height_)
	{
		if (hasNodeInCloseList(target_pos))
		{
			return false;
		}

		if (abs(current_pos.y + current_pos.x - target_pos.y - target_pos.x) == 1)
		{
			return query_(target_pos);
		}
		else if (bAllowCorner)
		{
			return (canreach(Vec2(current_pos.x + target_pos.x - current_pos.x, current_pos.y))
				&& canreach(Vec2(current_pos.x, current_pos.y + target_pos.y - current_pos.y)));
		}
	}
	return false;
}

void JOAStar::findCanreachPos(const Vec2 &current_pos, std::vector<Vec2> &canreach_pos)
{
	Vec2 target_pos;
	canreach_pos.clear();
	int row_index = current_pos.y - 1;
	const int max_row = current_pos.y + 1;
	const int max_col = current_pos.x + 1;

	if (row_index < 0)
	{
		row_index = 0;
	}
	
	while (row_index <= max_row)
	{
		int col_index = current_pos.x - 1;

		if (col_index < 0)
		{
			col_index = 0;
		}
	
		while (col_index <= max_col)
		{
			target_pos.set(col_index, row_index);
			if (canreach(current_pos, target_pos))
			{
				canreach_pos.push_back(target_pos);
			}
			++col_index;
		}
		++row_index;
	}
}

void JOAStar::handleFoundNode(Node *current_node, Node *target_node)
{
	unsigned int g_value = calculGValue(current_node, target_node->pos);
	if (g_value < target_node->g)
	{
		target_node->g = g_value;
		target_node->parent = current_node;

		size_t index = 0;
		if (getNodeIndex(target_node, index))
		{
			percolateUp(index);
		}
		else
		{
			assert(false);
		}
	}
}

void JOAStar::handleNotFoundNode(Node *current_node, Node *target_node, const Vec2 &end_pos)
{
	target_node->parent = current_node;
	target_node->h = calculHValue(target_node->pos, end_pos);
	target_node->g = calculGValue(current_node, target_node->pos);

	Node *&node_ptr = maps_[target_node->pos.y * height_ + target_node->pos.x];
	node_ptr = target_node;
	node_ptr->state = IN_OPENLIST;

	open_list_.push_back(target_node);
	std::push_heap(open_list_.begin(), open_list_.end(), [](const Node *a, const Node *b)->bool
	{
		return a->f() > b->f();
	});
}


std::deque<JOPoint> JOAStar::search(const JOPoint &start_pos, const JOPoint &end_pos)
{
    std::deque<JOPoint> paths;
	Vec2 sp(start_pos.x, start_pos.y);
	Vec2 ep(end_pos.x, end_pos.y);
	if ((ep.x >= 0 && ep.x < width_)
		&& (ep.y >= 0 && ep.y < height_)
		&& (sp.x >= 0 && sp.x < width_)
		&& (sp.y >= 0 && sp.y < height_))
    {
        if (!maps_.empty()){
            memset(&maps_[0], 0, sizeof(Node *) * maps_.size());
        }
        maps_.resize(width_ * height_, nullptr);
        
		std::vector<Vec2> nearby_nodes;
        nearby_nodes.reserve(bAllowCorner ? 8 : 4);
        
		/*
		起点放入开启列表
		*/
		Node *start_node = new Node(sp);
        open_list_.push_back(start_node);
        
		/*
		设置起点所对应节点的状态
		*/
        Node *&node_ptr = maps_[start_node->pos.y * height_ + start_node->pos.x];
        node_ptr = start_node;
        node_ptr->state = IN_OPENLIST;
        
        while (!open_list_.empty())
        {
			/*
			取出F值最小的节点
			*/
            Node *current_node = *open_list_.begin();
            std::pop_heap(open_list_.begin(), open_list_.end(), [](const Node *a, const Node *b)->bool
                          {
                              return a->f() > b->f();
                          });
            open_list_.pop_back();
            maps_[current_node->pos.y * height_ + current_node->pos.x]->state = IN_CLOSELIST;
            
			/*
			搜索附近可通行的位置
			*/
            findCanreachPos(current_node->pos, nearby_nodes);
            
            size_t index = 0;
            const size_t size = nearby_nodes.size();
            while (index < size)
            {
				/*
				如果存在于开启列表
				*/
                Node *new_node = nullptr;
                if (hasNoodeInOpenList(nearby_nodes[index], new_node))
                {
                    handleFoundNode(current_node, new_node);
                }
                else
                {
					/*
					如果不存在于开启列表
					*/
                    new_node = new Node(nearby_nodes[index]);
					handleNotFoundNode(current_node, new_node, ep);
                    
					/*
					找到终点
					*/
					if (nearby_nodes[index] == ep)
                    {
                        while (new_node->parent)
                        {
                            paths.push_front(JOPoint(new_node->pos.x, new_node->pos.y));
                            new_node = new_node->parent;
                        }
						std::reverse(paths.begin(), paths.end());
                        goto __end__;
                    }
                }
                ++index;
            }
        }
    }
    
__end__:
    clear();
    return paths;
}

NS_JOFW_END
