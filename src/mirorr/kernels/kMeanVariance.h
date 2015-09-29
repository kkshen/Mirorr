const std::string kMeanVarianceString =
"#ifndef __OPENCL_VERSION__ //defined by the kernel compiler\n"
"#define __kernel\n"
"#define __global\n"
"#define CLK_LOCAL_MEM_FENCE\n"
"#define uint unsigned int\n"
"#endif\n"
"\n"
"/**\n"
" *\n"
" * --- WARNING ---\n"
" * The .h version of this file is generated from the .cl file. You\n"
" * MUST run convertCl2Header.sh after any modification to the .cl file!\n"
" * --- WARNING ----\n"
" *\n"
" *\n"
" * Code by Jeremy Coatelen, 11 Apr 2011\n"
" * Commented and edited by D Rivest-Henault, 31 Jan 2013\n"
" * \n"
" * * Copyright (c) 2009-15 CSIRO. All rights reserved.\n"
" * \n"
" * For complete copyright, license and disclaimer of warranty\n"
" * information see the LICENSE.txt file for details.\n"
" * \n"
" * This software is distributed WITHOUT ANY WARRANTY; without even\n"
" * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
" * PURPOSE.  See the above copyright notice for more information.\n"
" * \n"
" */\n"
"\n"
"\n"
"/**\n"
" * kMeanVariance:\n"
" * This OpenCL kernel takes two image buffers and compute the mean\n"
" * and variance of the BlockWidth^N, N in{2,3}, with starting index\n"
" * defined by \"ig = get_global_id(0)\".\n"
" *\n"
" * @param inputL Base image buffer\n"
" * @param inputR Search image buffer\n"
" * @param outputLMean Output mean buffer for the inputL image\n"
" * @param outputRMean Output mean buffer for the inputR image\n"
" * @param outputLVariance Output variance buffer for the inputL image\n"
" * @param outputRVariance Output variance buffer for the inputR image\n"
" * @param w Width of the search image buffer\n"
" * @param h Height of the search image buffer\n"
" * @param d Depth of the search image buffer\n"
" * @param BlockWidth size of the {2,3}D block\n"
" */\n"
"\n"
"//__attribute__((vec_type_hint(float4)))\n"
"//#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel\n"
"void kMeanVariance(\n"
"   __global float * inputL,    __global float * inputR,\n"
"   __global float * outputLMean,     __global float * outputRMean,\n"
"   __global float * outputLVariance,   __global float * outputRVariance,\n"
"   const uint w, const uint h, const uint d,\n"
"   uint BlockWidth)\n"
"{\n"
"  int ig = get_global_id(0);\n"
"#if 0\n"
"  float   max1  = 0.0; float max2  = 0.0;\n"
"  float   mean1 = 0.0; float mean2 = 0.0;\n"
"  float   normalisation;\n"
"#else\n"
"  double   max1  = 0.0; double max2  = 0.0;\n"
"  double   mean1 = 0.0; double mean2 = 0.0;\n"
"  double   normalisation;\n"
"#endif\n"
"  int     i = 0; int j = 0; int k = 0;\n"
"  int     I; int J; int K;\n"
"  int     wh = w * h;\n"
"  float   diff;\n"
"  int     tmp; int n;\n"
"  int     BlockWidth2 = BlockWidth*BlockWidth;\n"
"  int     BlockWidth3 = BlockWidth2*BlockWidth;\n"
"\n"
"  if(ig < wh*d)\n"
"  {\n"
"    // Initialization of the matrices (edges)\n"
"    normalisation = (float)((d>1)?(BlockWidth3):(BlockWidth2));\\\n"
"    outputLMean[ig] = 1.0/normalisation;\n"
"    outputRMean[ig] = 1.0/normalisation;\n"
"    outputLVariance[ig] = 1.0/normalisation;\n"
"    outputRVariance[ig] = 1.0/normalisation;\n"
"\n"
"    // Set the indexes\n"
"    i = ig % wh;\n"
"    I = i % w;      //  I in [ 0 ... WIDTH  ]\n"
"    J = i / w;      //  J in [ 0 ... HEIGHT ]\n"
"    K = ig / wh;    //  K in [ 0 ... DEPTH  ]\n"
"\n"
"    // Check the edges\n"
"    if( I > w-BlockWidth)           return;\n"
"    if( J > h-BlockWidth)           return;\n"
"    if( d>1 && (K > d-BlockWidth))  return;\n"
"    if( d < 1 )                     return;\n"
"    mean1 = 0.0; mean2 = 0.0;\n"
"    max1  = 0.0; max2  = 0.0;\n"
"\n"
"    // Compute the Mean matrix\n"
"    if( d>1 )\n"
"    {\n"
"      for(n = 0 ; n < BlockWidth3; ++n)\n"
"      {\n"
"        // Set the moving indexes in the block of the current neigh point\n"
"        tmp    = n % BlockWidth2;\n"
"        i      = I + tmp % BlockWidth;\n"
"        j      = J + tmp / BlockWidth;\n"
"        k      = K + n / BlockWidth2;\n"
"        mean1 += inputL[k * wh + j * w + i];\n"
"        mean2 += inputR[k * wh + j * w + i];\n"
"      }\n"
"    }\n"
"    else\n"
"    {\n"
"      for(n = 0 ; n < BlockWidth2; ++n)\n"
"      {\n"
"        // Set the moving indexes in the block of the current neigh point\n"
"        i      = I + n % BlockWidth;\n"
"        j      = J + n / BlockWidth;\n"
"        mean1 += inputL[j * w + i];\n"
"        mean2 += inputR[j * w + i];\n"
"      }\n"
"    }\n"
"    mean1 = mean1 / normalisation;\n"
"    mean2 = mean2 / normalisation;\n"
"    outputLMean[ig] = (float)mean1;\n"
"    outputRMean[ig] = (float)mean2;\n"
"\n"
"    // Compute the Variance matrix\n"
"    if( d>1 )\n"
"    {\n"
"     for(k = K ; k <= K+BlockWidth; k++)\n"
"       for(j = J ; j <= J+BlockWidth; j++)\n"
"         for(i = I ; i <= I+BlockWidth; i++)\n"
"           {\n"
"             max1 += (inputL[k * wh + j * w + i] - mean1)*(inputL[k * wh + j * w + i] - mean1);\n"
"             max2 += (inputR[k * wh + j * w + i] - mean2)*(inputR[k * wh + j * w + i] - mean2);\n"
"           }\n"
"    }\n"
"    else\n"
"    {\n"
"      for(j = J ; j <= J+BlockWidth; j++)\n"
"        for(i = I ; i <= I+BlockWidth; i++)\n"
"        {\n"
"          max1 += (inputL[j * w + i] - mean1)*(inputL[j * w + i] - mean1);\n"
"          max2 += (inputR[j * w + i] - mean2)*(inputR[j * w + i] - mean2);\n"
"        }\n"
"    }\n"
"    max1 = max1 / normalisation;\n"
"    max2 = max2 / normalisation;\n"
"    outputLVariance[ig] = (float)(max1);\n"
"    outputRVariance[ig] = (float)(max2);\n"
"  }\n"
"\n"
"  // Finish the threads\n"
"  barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
;