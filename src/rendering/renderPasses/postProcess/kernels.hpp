#pragma once

static constexpr float blurKernel[9] = {
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
};

static constexpr float edgeKernel[9] = {
	1, 1, 1,
	1, -8, 1,
	1, 1, 1
};

static constexpr float sharpenKernel[9] = {
	-1, -1, -1,
	-1, 9, -1,
	-1, -1, -1
};