// Example demonstrating advanced Jsonnet features for PSO configuration
// This showcases the power of "JSON on steroids" with variables, functions, conditionals, etc.

local effects = import "effects.libsonnet";

// Configuration variables (can be set via external variables)
local buildConfig = {
  isDebug: std.extVar("BUILD_TYPE") == "Debug",
  enableProfiling: std.extVar("ENABLE_PROFILING") == "true", 
  targetPlatform: std.extVar("TARGET_PLATFORM"), // "PC", "Console", "Mobile"
  qualityLevel: std.parseJson(std.extVar("QUALITY_LEVEL")) // 0-3
};

// Platform-specific settings
local platformSettings = {
  PC: {
    maxRenderTargets: 8,
    supportsMSAA: true,
    supportsConservativeRaster: true,
    defaultMSAASamples: 4
  },
  Console: {
    maxRenderTargets: 8, 
    supportsMSAA: true,
    supportsConservativeRaster: true,
    defaultMSAASamples: 2
  },
  Mobile: {
    maxRenderTargets: 4,
    supportsMSAA: false,
    supportsConservativeRaster: false,
    defaultMSAASamples: 1
  }
};

local platform = platformSettings[buildConfig.targetPlatform];

// Quality-based settings (0=Low, 1=Medium, 2=High, 3=Ultra)
local qualitySettings = [
  { // Low quality
    msaaEnabled: false,
    msaaSamples: 1,
    useHalfResEffects: true,
    shadowQuality: "low"
  },
  { // Medium quality  
    msaaEnabled: platform.supportsMSAA,
    msaaSamples: std.min(2, platform.defaultMSAASamples),
    useHalfResEffects: false,
    shadowQuality: "medium"
  },
  { // High quality
    msaaEnabled: platform.supportsMSAA,
    msaaSamples: platform.defaultMSAASamples,
    useHalfResEffects: false, 
    shadowQuality: "high"
  },
  { // Ultra quality
    msaaEnabled: platform.supportsMSAA,
    msaaSamples: platform.defaultMSAASamples,
    useHalfResEffects: false,
    shadowQuality: "ultra"
  }
];

local quality = qualitySettings[buildConfig.qualityLevel];

// Helper function to create PSO with platform/quality awareness
local createOptimizedPSO(basePSO, overrides={}) = 
  local finalPSO = basePSO + overrides;
  
  // Apply platform-specific optimizations
  finalPSO + {
    rasterizerDesc: finalPSO.rasterizerDesc + {
      multisampleEnabled: quality.msaaEnabled,
      forcedSampleCount: if quality.msaaEnabled then quality.msaaSamples else 0,
      conservativeRasterEnabled: if platform.supportsConservativeRaster 
                                then finalPSO.rasterizerDesc.conservativeRasterEnabled
                                else false
    },
    
    blendDesc: finalPSO.blendDesc + {
      alphaToCoverageEnabled: quality.msaaEnabled && finalPSO.blendDesc.alphaToCoverageEnabled
    },
    
    // Limit render targets based on platform
    renderTargetCount: std.min(finalPSO.renderTargetCount, platform.maxRenderTargets)
  };

// Conditional shader variants based on configuration
local getShaderVariant(baseName, stage) = 
  local variants = [];
  local finalVariants = variants +
    (if buildConfig.isDebug then ["_DEBUG"] else []) +
    (if buildConfig.enableProfiling then ["_PROFILING"] else []) +
    (if quality.msaaEnabled then ["_MSAA"] else []) +
    (if quality.useHalfResEffects then ["_HALFRES"] else []);
  
  baseName + std.join("", finalVariants) + "." + stage;

// Macro-like function for common render state combinations
local createBlendMode(mode) = 
  if mode == "opaque" then
    effects.RtBlendState(blendEnabled=false)
  else if mode == "alpha" then
    effects.RtBlendState(
      blendEnabled=true,
      srcRgb=effects.BlendFactors.SrcAlpha,
      dstRgb=effects.BlendFactors.OneMinusSrcAlpha,
      srcAlpha=effects.BlendFactors.One,
      dstAlpha=effects.BlendFactors.OneMinusSrcAlpha
    )
  else if mode == "additive" then
    effects.RtBlendState(
      blendEnabled=true,
      srcRgb=effects.BlendFactors.SrcAlpha,
      dstRgb=effects.BlendFactors.One,
      srcAlpha=effects.BlendFactors.Zero,
      dstAlpha=effects.BlendFactors.One
    )
  else if mode == "multiply" then
    effects.RtBlendState(
      blendEnabled=true,
      srcRgb=effects.BlendFactors.Zero,
      dstRgb=effects.BlendFactors.SrcColor,
      srcAlpha=effects.BlendFactors.Zero,
      dstAlpha=effects.BlendFactors.SrcAlpha
    )
  else
    error "Unknown blend mode: " + mode;

{
  // Export configuration for debugging
  _config: {
    build: buildConfig,
    platform: platform,
    quality: quality
  },

  // Standard forward rendering pass
  forwardPass: createOptimizedPSO(effects.default, {
    vs: getShaderVariant("forward", "vs"),
    ps: getShaderVariant("forward", "ps"),
    
    blendDesc: effects.default.blendDesc + {
      rtBlend: [createBlendMode("opaque")] + 
               [effects.RtBlendState() for i in std.range(1, 7)]
    }
  }),

  // Transparent objects pass  
  transparentPass: createOptimizedPSO(effects.default, {
    vs: getShaderVariant("transparent", "vs"),
    ps: getShaderVariant("transparent", "ps"),
    
    // Sort back-to-front, no depth write
    depthStencilDesc: effects.default.depthStencilDesc + {
      depthAccess: effects.DepthAccess.Read
    },
    
    blendDesc: effects.default.blendDesc + {
      rtBlend: [createBlendMode("alpha")] +
               [effects.RtBlendState() for i in std.range(1, 7)]
    }
  }),

  // Particle system with different blend modes
  particles: {
    [mode]: createOptimizedPSO(effects.default, {
      vs: getShaderVariant("particle", "vs"),
      ps: getShaderVariant("particle", "ps"),
      
      primitiveTopology: effects.PrimitiveTopologies.TriangleStrip,
      
      rasterizerDesc: effects.default.rasterizerDesc + {
        cullMode: effects.CullModes.None
      },
      
      depthStencilDesc: effects.default.depthStencilDesc + {
        depthAccess: effects.DepthAccess.Read // No depth writes for particles
      },
      
      blendDesc: effects.default.blendDesc + {
        rtBlend: [createBlendMode(mode)] +
                 [effects.RtBlendState() for i in std.range(1, 7)]
      }
    })
    for mode in ["alpha", "additive", "multiply"]
  },

  // Shadow mapping with cascaded shadow maps
  shadowMapping: {
    local shadowBias = {
      low: { depth: 500, slope: 1.0 },
      medium: { depth: 1000, slope: 2.0 },
      high: { depth: 2000, slope: 3.0 },
      ultra: { depth: 4000, slope: 4.0 }
    }[quality.shadowQuality],
    
    depth: createOptimizedPSO(effects.default + effects.presets.depthWriteOnly, {
      vs: getShaderVariant("shadow", "vs"),
      ps: null, // Depth-only
      
      renderTargetCount: 0,
      renderTargetFormats: ["Unknown" for i in std.range(0, 7)],
      depthStencilFormat: if quality.shadowQuality == "ultra" then "D32Float" else "D24UnormS8Uint",
      
      rasterizerDesc: effects.default.rasterizerDesc + {
        cullMode: effects.CullModes.Front, // Reduce shadow acne
        depthBias: shadowBias.depth,
        slopeScaledDepthBias: shadowBias.slope
      }
    })
  },

  // Deferred rendering G-Buffer
  deferredGBuffer: createOptimizedPSO(effects.default, {
    vs: getShaderVariant("gbuffer", "vs"),
    ps: getShaderVariant("gbuffer", "ps"),
    
    renderTargetCount: std.min(4, platform.maxRenderTargets),
    renderTargetFormats: 
      if platform.maxRenderTargets >= 4 then [
        "RGBA8Unorm",     // Albedo + metallic
        "RGBA16Float",    // Normal + roughness
        "RGBA8Unorm",     // Motion + AO  
        "R11G11B10Float"  // Emission
      ] + ["Unknown" for i in std.range(4, 7)]
      else [
        "RGBA8Unorm",     // Albedo + metallic
        "RGBA8Unorm",     // Normal(xy) + roughness + metallic
        "Unknown",
        "Unknown"
      ] + ["Unknown" for i in std.range(4, 7)],
    
    blendDesc: effects.default.blendDesc + {
      rtBlend: [
        effects.RtBlendState(blendEnabled=false) 
        for i in std.range(0, 7)
      ]
    }
  }),

  // Debug visualizations
  debug: {
    wireframe: if buildConfig.isDebug then
      createOptimizedPSO(effects.default + effects.presets.wireframe, {
        vs: getShaderVariant("debug", "vs"),
        ps: getShaderVariant("debug", "ps")
      })
    else null,
    
    normals: if buildConfig.isDebug then
      createOptimizedPSO(effects.default, {
        vs: getShaderVariant("debug_normals", "vs"),
        ps: getShaderVariant("debug_normals", "ps"),
        primitiveTopology: effects.PrimitiveTopologies.LineList
      })
    else null
  }
}

