#include "limb_config.h"

LimbKinematicsModel* limbs[] = {
    &limb,
    &limb,
    &limb,
    &limb,
    &limb,
    &limb,
};
LimbConfig_t limb_config[] = {
    {   // left legs
        .displacement=Vector3D(-30, 110, 0),
        .orientation=Vector3D(0, 0, M_PI),
    },
    {
        .displacement=Vector3D(-30, 0, 0),
        .orientation=Vector3D(0, 0, M_PI),
    },
    {
        .displacement=Vector3D(-30, -90, 0),
        .orientation=Vector3D(0, 0, M_PI),
    },
    {   // right legs
        .displacement=Vector3D(30, 110, 0),
        .orientation=Vector3D(0, 0, 0),
    },
    {
        .displacement=Vector3D(30, 0, 0),
        .orientation=Vector3D(0, 0, 0),
    },
    {
        .displacement=Vector3D(30, -90, 0),
        .orientation=Vector3D(0, 0, 0),
    },
};
Vector3D default_position(0, 20, -24);
Vector3D default_orientation(0.0);
ConstraintVolume working_space(0, -30, -30, 0, 30, 30);