//
// Copyright © 2017 Arm Ltd. All rights reserved.
// See LICENSE file in the project root for full license information.
//

#include "ClConvolution2dUint8Workload.hpp"
#include "backends/ClTensorHandle.hpp"
#include "backends/CpuTensorHandle.hpp"
#include "backends/ArmComputeTensorUtils.hpp"
#include "backends/ClLayerSupport.hpp"

namespace armnn
{
using namespace armcomputetensorutils;

ClConvolution2dUint8Workload::ClConvolution2dUint8Workload(const Convolution2dQueueDescriptor& descriptor,
    const WorkloadInfo& info, std::shared_ptr<arm_compute::MemoryManagerOnDemand>& memoryManager)
    : Uint8Workload<Convolution2dQueueDescriptor>(descriptor, info)
    , m_ConvolutionLayer(memoryManager)
{
    // todo: check tensor shapes match
    const TensorInfo& weightInfo = m_Data.m_Weight->GetTensorInfo();

    m_KernelTensor = std::make_unique<arm_compute::CLTensor>();
    BuildArmComputeTensor(*m_KernelTensor, weightInfo);

    arm_compute::PadStrideInfo padStrideInfo(m_Data.m_Parameters.m_StrideX,
                                             m_Data.m_Parameters.m_StrideY,
                                             m_Data.m_Parameters.m_PadLeft,
                                             m_Data.m_Parameters.m_PadRight,
                                             m_Data.m_Parameters.m_PadTop,
                                             m_Data.m_Parameters.m_PadBottom,
                                             arm_compute::DimensionRoundingType::FLOOR);

    if (m_Data.m_Parameters.m_BiasEnabled)
    {
        m_BiasTensor = std::make_unique<arm_compute::CLTensor>();
        BuildArmComputeTensor(*m_BiasTensor, m_Data.m_Bias->GetTensorInfo());
    }

    m_Data.ValidateInputsOutputs("ClConvolution2dUint8Workload", 1, 1);

    arm_compute::ICLTensor& input  = static_cast<IClTensorHandle*>(m_Data.m_Inputs[0])->GetTensor();
    arm_compute::ICLTensor& output = static_cast<IClTensorHandle*>(m_Data.m_Outputs[0])->GetTensor();

    m_ConvolutionLayer.configure(&input,
                                 m_KernelTensor.get(),
                                 m_BiasTensor.get(),
                                 &output,
                                 padStrideInfo);

    InitialiseArmComputeClTensorData(*m_KernelTensor, m_Data.m_Weight->GetConstTensor<uint8_t>());

    if (m_BiasTensor)
    {
        InitialiseArmComputeClTensorData(*m_BiasTensor, m_Data.m_Bias->GetConstTensor<int32_t>());
    }

    // Force Compute Library to perform the necessary copying and reshaping, after which
    // delete all the input tensors that will no longer be needed
    m_ConvolutionLayer.prepare();
    FreeUnusedTensors();
}

void ClConvolution2dUint8Workload::Execute() const
{
    ARMNN_SCOPED_PROFILING_EVENT_CL("ClConvolution2dUint8Workload_Execute");

    m_ConvolutionLayer.run();
}

void ClConvolution2dUint8Workload::FreeUnusedTensors()
{
    FreeTensorIfUnused(m_KernelTensor);
    FreeTensorIfUnused(m_BiasTensor);
}

} //namespace armnn

