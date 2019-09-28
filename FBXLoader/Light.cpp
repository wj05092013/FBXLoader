#include "stdafx.h"

ba::DirectionalLight::DirectionalLight() :
	ambient(0.0f, 0.0f, 0.0f, 0.0f),
	diffuse(0.0f, 0.0f, 0.0f, 0.0f),
	specular(0.0f, 0.0f, 0.0f, 0.0f),
	direction(0.0f, 0.0f, 0.0f),
	pad(0.0f)
{
}

ba::PointLight::PointLight() :
	ambient(0.0f, 0.0f, 0.0f, 0.0f),
	diffuse(0.0f, 0.0f, 0.0f, 0.0f),
	specular(0.0f, 0.0f, 0.0f, 0.0f),
	pos(0.0f, 0.0f, 0.0f),
	range(0.0f),
	attenuation(0.0f, 0.0f, 0.0f),
	pad(0.0f)
{
}

ba::SpotLight::SpotLight() :
	ambient(0.0f, 0.0f, 0.0f, 0.0f),
	diffuse(0.0f, 0.0f, 0.0f, 0.0f),
	specular(0.0f, 0.0f, 0.0f, 0.0f),
	pos(0.0f, 0.0f, 0.0f),
	range(0.0f),
	direction(0.0f, 0.0f, 0.0f),
	spot(0.0f),
	attenuation(0.0f, 0.0f, 0.0f),
	pad(0.0f)
{
}

ba::Material::Material() :
	ambient(0.0f, 0.0f, 0.0f, 0.0f),
	diffuse(0.0f, 0.0f, 0.0f, 0.0f),
	specular(0.0f, 0.0f, 0.0f, 0.0f),
	reflection(0.0f, 0.0f, 0.0f, 0.0f)
{
}
