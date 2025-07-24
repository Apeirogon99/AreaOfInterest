#pragma once

#include <memory>

#include "Common/Game/Entity.h"

class AOI
{
public:
	AOI();
	~AOI();

public:
	void UpdateVisibleDistance(const std::unique_ptr<Entity>& Entity);

public:
	
};