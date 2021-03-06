
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014-2015 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// appleseed.renderer headers.
#include "renderer/global/globaltypes.h"
#include "renderer/kernel/shading/shadingray.h"
#include "renderer/modeling/camera/camera.h"
#include "renderer/modeling/camera/sphericalcamera.h"
#include "renderer/modeling/project/project.h"
#include "renderer/utility/paramarray.h"

// appleseed.foundation headers.
#include "foundation/math/vector.h"
#include "foundation/utility/autoreleaseptr.h"
#include "foundation/utility/iostreamop.h"
#include "foundation/utility/test.h"

using namespace foundation;
using namespace renderer;

TEST_SUITE(Renderer_Modeling_Camera_SphericalCamera)
{
    TEST_CASE(ProjectPoint)
    {
        auto_release_ptr<Project> project(ProjectFactory::create("test"));

        SphericalCameraFactory factory;
        auto_release_ptr<Camera> camera(factory.create("camera", ParamArray()));

        camera->on_frame_begin(project.ref());

        SamplingContext::RNGType rng;
        SamplingContext sampling_context(rng, SamplingContext::QMCMode);

        ShadingRay ray;
        camera->generate_ray(sampling_context, Dual2d(Vector2d(1.0, 1.0)), ray);

        const Vector3d hit_point = ray.m_org + 3.0 * normalize(ray.m_dir);

        Vector2d projected;
        const bool success = camera->project_point(0.0, hit_point, projected);

        ASSERT_TRUE(success);
        EXPECT_FEQ(Vector2d(1.0, 1.0), projected);

        camera->on_frame_end(project.ref());
    }
}
