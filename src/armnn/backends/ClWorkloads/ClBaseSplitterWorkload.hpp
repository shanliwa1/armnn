//
// Copyright © 2017 Arm Ltd. All rights reserved.
// See LICENSE file in the project root for full license information.
//

#pragma once

#include "backends/ClWorkloadUtils.hpp"

namespace armnn
{

// Base class template providing an implementation of the Splitter layer common to all data types.
template <armnn::DataType... DataTypes>
class ClBaseSplitterWorkload : public TypedWorkload<SplitterQueueDescriptor, DataTypes...>
{
public:
    using TypedWorkload<SplitterQueueDescriptor, DataTypes...>::TypedWorkload;

    void Execute() const override
    {
        // With subtensors, merger is a no-op.
    }
};

} //namespace armnn
