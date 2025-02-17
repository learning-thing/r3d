// The output returns the actual distances between the center of the cube and the fragments
// This shader requires a depth buffer and a color attachment
// Used for shadow mapping for omni lights

#version 330 core

in vec3 vPosition;

uniform vec3 uViewPosition;

out float FragDistance;

void main()
{
    FragDistance = length(vPosition - uViewPosition);
}
