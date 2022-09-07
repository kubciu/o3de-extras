/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Debug/Timer.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/Math/Color.h>
#include <AzCore/std/string/conversions.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <ONNX/ONNXBus.h>
#include <onnxruntime_cxx_api.h>

namespace ONNX
{
    //! Generic ONNX model which can be used to create an inference session and run inferences.
    class Model
    {
    public:
        Model() = default;

        //! Required params to create session and run inference, passed to Load().
        struct InitSettings
        {
            //! Source of onnx model file.
            AZ::IO::FixedMaxPath m_modelFile;
            AZStd::string m_modelName = ""; //!< Used to create groupings for ImGui dashboard graphs in editor, idea is that the inference runtimes from the same model instance get displayed on the same graph.
            AZ::Color m_modelColor = AZ::Color::CreateFromRgba(229, 56, 59, 255); //!< Sets the color of the dashboard graph for runtime statistics of this model instance.
            bool m_cudaEnable = false; //!< Toggle to create a CUDA session on gpu, if disabled normal cpu session created.
        };

        //! Initialises necessary params in order to run inference.
        //! Must be executed before Run().
        //! Creates the session, memory info, and extracts input and output names, shapes and count from onnx model file.
        //! Creates vector m_outputs that holds inference outputs based on onnx model file's output counts and shapes for each output
        //! ie if onnx model has output count of 2 each with shapes of {1, 1, 28, 28} then m_outputs will hold 2 vectors of length 784 each.
        //! Only needs to be executed once, inferences using the same onnx model file can be run by providing different input/output params to
        //! Run().
        void Load(const InitSettings& initSettings);

        //! Executes the inference using the loaded model.
        //! Input and output vectors are used to generate their respective tensors.
        //! Output is mutated directly.
        void Run(AZStd::vector<AZStd::vector<float>>& inputs);

        float m_delta = 0.0f; //!< Runtime in ms of latest inference.
        AZStd::vector<AZStd::vector<float>> m_outputs; //!< Outputs of latest inference (initialized by Load() - size and shape extracted from onnx model file).

    protected:
        bool m_cudaEnable = false; // Determines if inferencing of the model instance will be run on gpu using CUDA (run on CPU by default).
        AZStd::string m_modelName; // Used to create groupings for ImGui dashboard graphs in editor, idea is that the inference runtimes from the same model instance get displayed on the same graph.
        AZ::Color m_modelColor = AZ::Color::CreateFromRgba(229, 56, 59, 255); // Sets the color of the dashboard graph for runtime statistics of this model instance.
        AZ::Debug::Timer m_timer; // Timer instance that is used within Run() to calculate inference runtime, and obtain the value in m_delta.
        Ort::MemoryInfo m_memoryInfo{ nullptr }; // Created by Load() and holds information about the memory allocator used by the instance and the memory type. These are set to OrtDeviceAllocator and OrtMemTypeCpu for both CPU and GPU execution (contrary to how it may seem this is the correct MemType for CUDA as well).
        Ort::Session m_session{ nullptr }; // Created by Load(), and is unique to the model.onnx file used - created using the Ort::Env and SessionOptions which are used to specify CPU or CUDA execution.
        AZStd::vector<AZStd::vector<int64_t>> m_inputShapes; // A vector of the dimensions of input, eg vectors specifying dimension and magnitude of dimension such as { 1, 1, 28, 28 }.
        size_t m_inputCount = 0; // The number of inputs in the model.onnx file. Corresponds with the number of input names.
        AZStd::vector<const char*> m_inputNames; // A vector of the input names extracted from the model.onnx file.
        AZStd::vector<AZStd::vector<int64_t>> m_outputShapes; // A vector of the dimensions of output, eg vectors specifying dimension and magnitude of dimension such as { 1, 10 }.
        size_t m_outputCount = 0; // The number of outputs in the model.onnx file. Corresponds with the number of output names.
        AZStd::vector<const char*> m_outputNames; // A vector of the output names extracted from the model.onnx file.
    };
} // namespace ONNX
