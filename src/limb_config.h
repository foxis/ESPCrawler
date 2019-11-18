
PlanarJoint_t joints_2dof[] = {
    {
        .constraints = {
            .min = -M_PI / 4.0, 
            .max = M_PI / 4.0
        }, 
        .length = 26 
    },
    {
        .constraints = {
            .min = -M_PI / 4.0, 
            .max = M_PI / 4.0
        }, 
        .length = 30
    },
};
ConstraintVolume vol_2dof(Vector3D(-30, -73/2.0, -40/2.0), Vector3D(0, 73/2.0, 40/2.0));

PlanarKinematics2DOF limb(joints_2dof, vol_2dof);
