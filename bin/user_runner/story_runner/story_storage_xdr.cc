// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "peridot/bin/user_runner/story_runner/story_storage_xdr.h"

#include "lib/fidl/cpp/clone.h"

namespace fuchsia {
namespace modular {

void XdrLinkPath(XdrContext* const xdr, LinkPath* const data) {
  xdr->Field("module_path", &data->module_path);
  xdr->Field("link_name", &data->link_name);
}

void XdrChainKeyToLinkData(XdrContext* const xdr,
                           ChainKeyToLinkData* const data) {
  xdr->Field("key", &data->key);
  xdr->Field("link_path", &data->link_path, XdrLinkPath);
}

void XdrChainData(XdrContext* const xdr, ChainData* const data) {
  xdr->Field("key_to_link_map", &data->key_to_link_map, XdrChainKeyToLinkData);
}

void XdrSurfaceRelation(XdrContext* const xdr, SurfaceRelation* const data) {
  xdr->Field("arrangement", &data->arrangement);
  xdr->Field("dependency", &data->dependency);
  xdr->Field("emphasis", &data->emphasis);
}

void XdrIntentParameterData(XdrContext* const xdr,
                            IntentParameterData* const data) {
  static constexpr char kTag[] = "tag";
  static constexpr char kEntityReference[] = "entity_reference";
  static constexpr char kJson[] = "json";
  static constexpr char kEntityType[] = "entity_type";
  static constexpr char kLinkName[] = "link_name";
  static constexpr char kLinkPath[] = "link_path";

  switch (xdr->op()) {
    case XdrOp::FROM_JSON: {
      std::string tag;
      xdr->Field(kTag, &tag);

      if (tag == kEntityReference) {
        fidl::StringPtr value;
        xdr->Field(kEntityReference, &value);
        data->set_entity_reference(std::move(value));
      } else if (tag == kJson) {
        fidl::StringPtr value;
        xdr->Field(kJson, &value);
        data->set_json(std::move(value));
      } else if (tag == kEntityType) {
        ::fidl::VectorPtr<::fidl::StringPtr> value;
        xdr->Field(kEntityType, &value);
        data->set_entity_type(std::move(value));
      } else if (tag == kLinkName) {
        fidl::StringPtr value;
        xdr->Field(kLinkName, &value);
        data->set_link_name(std::move(value));
      } else if (tag == kLinkPath) {
        LinkPath value;
        xdr->Field(kLinkPath, &value, XdrLinkPath);
        data->set_link_path(std::move(value));
      } else {
        FXL_LOG(ERROR) << "XdrIntentParameterData FROM_JSON unknown tag: "
                       << tag;
      }
      break;
    }

    case XdrOp::TO_JSON: {
      std::string tag;

      // The unusual call to operator->() in the cases below is because
      // operator-> for all of FIDL's pointer types to {strings, arrays,
      // structs} returns a _non-const_ reference to the inner pointer,
      // which is required by the xdr->Field() method. Calling get() returns
      // a const pointer for arrays and strings. get() does return a non-const
      // pointer for FIDL structs, but given that operator->() is required for
      // some FIDL types, we might as well be consistent and use operator->()
      // for all types.

      switch (data->Which()) {
        case IntentParameterData::Tag::kEntityReference: {
          tag = kEntityReference;
          fidl::StringPtr value = data->entity_reference();
          xdr->Field(kEntityReference, &value);
          break;
        }
        case IntentParameterData::Tag::kJson: {
          tag = kJson;
          fidl::StringPtr value = data->json();
          xdr->Field(kJson, &value);
          break;
        }
        case IntentParameterData::Tag::kEntityType: {
          tag = kEntityType;
          fidl::VectorPtr<fidl::StringPtr> value = Clone(data->entity_type());
          xdr->Field(kEntityType, &value);
          break;
        }
        case IntentParameterData::Tag::kLinkName: {
          tag = kLinkName;
          fidl::StringPtr value = data->link_name();
          xdr->Field(kLinkName, &value);
          break;
        }
        case IntentParameterData::Tag::kLinkPath: {
          tag = kLinkPath;
          xdr->Field(kLinkPath, &data->link_path(), XdrLinkPath);
          break;
        }
        case IntentParameterData::Tag::Invalid:
          FXL_LOG(ERROR) << "XdrIntentParameterData TO_JSON unknown tag: "
                         << static_cast<int>(data->Which());
          break;
      }

      xdr->Field(kTag, &tag);
      break;
    }
  }
}

void XdrIntentParameter(XdrContext* const xdr, IntentParameter* const data) {
  xdr->Field("name", &data->name);
  xdr->Field("data", &data->data, XdrIntentParameterData);
}

void XdrIntent(XdrContext* const xdr, Intent* const data) {
  xdr->Field("action_name", &data->action.name);
  xdr->Field("action_handler", &data->action.handler);
  xdr->Field("parameters", &data->parameters, XdrIntentParameter);
}

void XdrParameterConstraint(XdrContext* const xdr,
                            ParameterConstraint* const data) {
  xdr->Field("name", &data->name);
  xdr->Field("type", &data->type);
}

void XdrModuleManifest(XdrContext* const xdr, ModuleManifest* const data) {
  xdr->Field("binary", &data->binary);
  xdr->Field("suggestion_headline", &data->suggestion_headline);
  xdr->Field("action", &data->action);
  xdr->Field("parameters", &data->parameter_constraints,
             XdrParameterConstraint);
  xdr->Field("composition_pattern", &data->composition_pattern);
}

void XdrModuleData_v1(XdrContext* const xdr, ModuleData* const data) {
  xdr->Field("url", &data->module_url);
  xdr->Field("module_path", &data->module_path);
  xdr->Field("module_source", &data->module_source);
  xdr->Field("surface_relation", &data->surface_relation, XdrSurfaceRelation);
  xdr->Field("module_stopped", &data->module_stopped);
  xdr->Field("intent", &data->intent, XdrIntent);

  // In previous versions we did not have these fields.
  data->chain_data.key_to_link_map.resize(0);
  data->module_manifest.reset();
}

void XdrModuleData_v2(XdrContext* const xdr, ModuleData* const data) {
  xdr->Field("url", &data->module_url);
  xdr->Field("module_path", &data->module_path);
  xdr->Field("module_source", &data->module_source);
  xdr->Field("surface_relation", &data->surface_relation, XdrSurfaceRelation);
  xdr->Field("module_stopped", &data->module_stopped);
  xdr->Field("intent", &data->intent, XdrIntent);
  xdr->Field("chain_data", &data->chain_data, XdrChainData);

  // In previous versions we did not have these fields.
  data->module_manifest.reset();
}

void XdrModuleData_v3(XdrContext* const xdr, ModuleData* const data) {
  xdr->Field("url", &data->module_url);
  xdr->Field("module_path", &data->module_path);
  xdr->Field("module_source", &data->module_source);
  xdr->Field("surface_relation", &data->surface_relation, XdrSurfaceRelation);
  xdr->Field("module_stopped", &data->module_stopped);
  xdr->Field("intent", &data->intent, XdrIntent);
  xdr->Field("chain_data", &data->chain_data, XdrChainData);
  xdr->Field("module_manifest", &data->module_manifest, XdrModuleManifest);
}

void XdrModuleData_v4(XdrContext* const xdr, ModuleData* const data) {
  if (!xdr->Version(4)) {
    return;
  }
  xdr->Field("url", &data->module_url);
  xdr->Field("module_path", &data->module_path);
  xdr->Field("module_source", &data->module_source);
  xdr->Field("surface_relation", &data->surface_relation, XdrSurfaceRelation);
  xdr->Field("module_stopped", &data->module_stopped);
  xdr->Field("intent", &data->intent, XdrIntent);
  xdr->Field("chain_data", &data->chain_data, XdrChainData);
  xdr->Field("module_manifest", &data->module_manifest, XdrModuleManifest);
}

XdrFilterType<ModuleData> XdrModuleData[] = {
    XdrModuleData_v4, XdrModuleData_v3, XdrModuleData_v2,
    XdrModuleData_v1, nullptr,
};

void XdrPerDeviceStoryInfo_v1(XdrContext* const xdr,
                              internal::PerDeviceStoryInfo* const data) {
  xdr->Field("device", &data->device_id);
  xdr->Field("id", &data->story_id);
  xdr->Field("time", &data->timestamp);
  xdr->Field("state", &data->state);
}

void XdrPerDeviceStoryInfo_v2(XdrContext* const xdr,
                              internal::PerDeviceStoryInfo* const data) {
  if (!xdr->Version(2)) {
    return;
  }
  xdr->Field("device", &data->device_id);
  xdr->Field("id", &data->story_id);
  xdr->Field("time", &data->timestamp);
  xdr->Field("state", &data->state);
}

XdrFilterType<internal::PerDeviceStoryInfo> XdrPerDeviceStoryInfo[] = {
    XdrPerDeviceStoryInfo_v2,
    XdrPerDeviceStoryInfo_v1,
    nullptr,
};

}  // namespace modular
}  // namespace fuchsia