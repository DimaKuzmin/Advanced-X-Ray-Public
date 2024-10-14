#include "StdAfx.h"
#include "Weapon.h"
#include "Actor.h"
#include "player_hud.h"
#include "GameMtlLib.h"


BOOL	b_hud_collision = FALSE;


// CURRENT HUD OFFSET IDX !!!! FOR AIMING
u8 CWeapon::GetCurrentHudOffsetIdx()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)		return 0;

	bool b_aiming = ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) ||
		(!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f));

	if (!b_aiming)
		return 0;
	else
	{
		if (!m_bAltZoomActive)
			return 1;
		else
			return 3;
	}
}
      
float CWeapon::GetControlInertionFactor() const
{
	float fInertionFactor = inherited::GetControlInertionFactor();
	if (IsScopeAttached() && IsZoomed())
		return m_fScopeInertionFactor;

	return fInertionFactor;
}
 
void _inertion(float& _val_cur, const float& _val_trgt, const float& _friction)
{
	float friction_i = 1.f - _friction;
	_val_cur = _val_cur * _friction + _val_trgt * friction_i;
}

float _lerp(const float& _val_a, const float& _val_b, const float& _factor)
{
	return (_val_a * (1.0 - _factor)) + (_val_b * _factor);
}

static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if (!result.O)
	{
		// �������� ����������� � ������ ��� ��������
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		if (T->material < GMLib.CountMaterial())
		{
			if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable) || GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flActorObstacle))
				return TRUE;
		}
	}
	*RQ = result;
	return FALSE;
}

static float GetRayQueryDist()
{
	collide::rq_result RQ;
	g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Device.vCameraDirection, 3.0f, collide::rqtStatic, RQ, Actor());
	if (!RQ.O)
	{
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + RQ.element;
		if (T->material < GMLib.CountMaterial())
		{
			collide::rq_result  RQ2;
			collide::rq_results RQR;
			RQ2.range = 3.0f;
			collide::ray_defs RD(Device.vCameraPosition, Device.vCameraDirection, RQ2.range, CDB::OPT_CULL, collide::rqtStatic);
			if (Level().ObjectSpace.RayQuery(RQR, RD, pick_trace_callback, &RQ2, NULL, Level().CurrentEntity()))
			{
				clamp(RQ2.range, RQ.range, RQ2.range);
				return RQ2.range;
			}
		}
	}
	return RQ.range;
}

void CWeapon::UpdateHudAdditional(Fmatrix& trans)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return;

	attachable_hud_item* hi = HudItemData();
	R_ASSERT(hi);

	u8 idx = GetCurrentHudOffsetIdx();

	//============= ������� ������ �� ����� ���� =============//
	if ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) ||
		(!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f))
	{
		Fvector						curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_hands_offset[0][idx];//pos,aim
		curr_rot = hi->m_measures.m_hands_offset[1][idx];//rot,aim
		curr_offs.mul(m_zoom_params.m_fZoomRotationFactor);
		curr_rot.mul(m_zoom_params.m_fZoomRotationFactor);

		Fmatrix						hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		Fmatrix						hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);

		if (pActor->IsZoomAimingMode())
			m_zoom_params.m_fZoomRotationFactor += Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;
		else
			m_zoom_params.m_fZoomRotationFactor -= Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;

		clamp(m_zoom_params.m_fZoomRotationFactor, 0.f, 1.f);
	}

	//============= �������� ������ =============//
	if (b_hud_collision)
	{
		float dist = GetRayQueryDist();

		Fvector curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_collision_offset[0];//pos,aim
		curr_rot = hi->m_measures.m_collision_offset[1];//rot,aim
		curr_offs.mul(m_fFactor);
		curr_rot.mul(m_fFactor);

		float m_fColPosition;
		float m_fColRotation;

		if (dist <= 0.8 && !IsZoomed())
		{
			m_fColPosition = curr_offs.y + ((1 - dist - 0.2) * 5.0f);
			m_fColRotation = curr_rot.x + ((1 - dist - 0.2) * 5.0f);
		}
		else
		{
			m_fColPosition = curr_offs.y;
			m_fColRotation = curr_rot.x;
		}

		if (m_fFactor < m_fColPosition)
		{
			m_fFactor += Device.fTimeDelta / 0.3;
			if (m_fFactor > m_fColPosition)
				m_fFactor = m_fColPosition;
		}
		else if (m_fFactor > m_fColPosition)
		{
			m_fFactor -= Device.fTimeDelta / 0.3;
			if (m_fFactor < m_fColPosition)
				m_fFactor = m_fColPosition;
		}

		Fmatrix hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		Fmatrix hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);

		clamp(m_fFactor, 0.f, 1.f);
	}
	else
	{
		m_fFactor = 0.0;
	}

	//============= �������������� ����� ���������� =============//

	clamp(idx, u8(0), u8(1));
	bool bForAim = (idx == 1);

	float fInertiaPower = GetInertionPowerFactor();

	float fYMag = pActor->fFPCamYawMagnitude;
	float fPMag = pActor->fFPCamPitchMagnitude;

	static float fAvgTimeDelta = Device.fTimeDelta;
	_inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	//======== ��������� ����������� ������� � ������� ========//
	if (!g_player_hud->inertion_allowed())
		return;

	//============= ������� ������ � ������� =============//
	float fStrafeMaxTime = m_strafe_offset[2][idx].y; // ����. ����� � ��������, �� ������� �� ���������� �� ������������ ���������
	if (fStrafeMaxTime <= EPS)
		fStrafeMaxTime = 0.01f;

	float fStepPerUpd = fAvgTimeDelta / fStrafeMaxTime; // �������� ��������� ������� ��������

	// ��������� ������� ������ �� �������� ������
	float fCamReturnSpeedMod = 1.5f; // ��������� �������� ������������ �������, ����������� �� �������� ������ (������ �� �����)
	// ����������� ����������� �������� �������� ������ ��� ������ �������
	float fStrafeMinAngle = _lerp(
		m_strafe_offset[3][0].y,
		m_strafe_offset[3][1].y,
		m_zoom_params.m_fZoomRotationFactor);

	// ����������� ����������� ������ �� �������� ������
	float fCamLimitBlend = _lerp(
		m_strafe_offset[3][0].x,
		m_strafe_offset[3][1].x,
		m_zoom_params.m_fZoomRotationFactor);

	// ������� ������ �� �������� ������
	if (abs(fYMag) > (m_fLR_CameraFactor == 0.0f ? fStrafeMinAngle : 0.0f))
	{ //--> ������ �������� �� ��� Y
		m_fLR_CameraFactor -= (fYMag * 0.025f);

		clamp(m_fLR_CameraFactor, -fCamLimitBlend, fCamLimitBlend);
	}
	else
	{ //--> ������ �� �������������� - ������� ������
		if (m_fLR_CameraFactor < 0.0f)
		{
			m_fLR_CameraFactor += fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, -fCamLimitBlend, 0.0f);
		}
		else
		{
			m_fLR_CameraFactor -= fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, 0.0f, fCamLimitBlend);
		}
	}
	// ��������� ������� ������ �� ������ ����
	float fChangeDirSpeedMod = 3; // ��������� ������ ������ ����������� ����������� �������, ���� ��� � ������ ������� �� ��������

	u32 iMovingState = pActor->MovingState();
	if ((iMovingState & mcLStrafe) != 0)
	{ // �������� �����
		float fVal = (m_fLR_MovingFactor > 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor -= fVal;
	}
	else if ((iMovingState & mcRStrafe) != 0)
	{ // �������� ������
		float fVal = (m_fLR_MovingFactor < 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor += fVal;
	}
	else
	{ // ��������� � ����� ������ ����������� - ������ ������� ������
		if (m_fLR_MovingFactor < 0.0f)
		{
			m_fLR_MovingFactor += fStepPerUpd;
			clamp(m_fLR_MovingFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_MovingFactor -= fStepPerUpd;
			clamp(m_fLR_MovingFactor, 0.0f, 1.0f);
		}
	}

	clamp(m_fLR_MovingFactor, -1.0f, 1.0f); // ������ ������� ������ �� ������ ��������� ��� ������

	// ��������� � ������������� �������� ������ �������
	float fLR_Factor = m_fLR_MovingFactor + (m_fLR_CameraFactor * fInertiaPower);
	clamp(fLR_Factor, -1.0f, 1.0f); // ������ ������� ������ �� ������ ��������� ��� ������

	// ���������� ������ ������ ��� ����������� ������ � ����
	for (int _idx = 0; _idx <= 1; _idx++)//<-- ��� �������� ��������
	{
		bool bEnabled = (m_strafe_offset[2][_idx].x != 0.0f);
		if (!bEnabled)
			continue;

		Fvector curr_offs, curr_rot;

		// �������� ������� ���� � �������
		curr_offs = m_strafe_offset[0][_idx]; //pos
		curr_offs.mul(fLR_Factor);                   // �������� �� ������ �������

		// ������� ���� � �������
		curr_rot = m_strafe_offset[1][_idx]; //rot
		curr_rot.mul(-PI / 180.f);                          // ����������� ���� � �������
		curr_rot.mul(fLR_Factor);                   // �������� �� ������ �������

		// ������ ������� ����� ������ \ ��������
		if (_idx == 0)
		{ // �� �����
			curr_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
			curr_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
		}
		else
		{ // �� ����� ����
			curr_offs.mul(m_zoom_params.m_fZoomRotationFactor);
			curr_rot.mul(m_zoom_params.m_fZoomRotationFactor);
		}

		Fmatrix hud_rotation;
		Fmatrix hud_rotation_y;

		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);
	}

	//============= ������� ������ =============//
   // ��������� �������
	float fInertiaSpeedMod = _lerp(
		hi->m_measures.m_inertion_params.m_tendto_speed,
		hi->m_measures.m_inertion_params.m_tendto_speed_aim,
		m_zoom_params.m_fZoomRotationFactor);

	float fInertiaReturnSpeedMod = _lerp(
		hi->m_measures.m_inertion_params.m_tendto_ret_speed,
		hi->m_measures.m_inertion_params.m_tendto_ret_speed_aim,
		m_zoom_params.m_fZoomRotationFactor);

	float fInertiaMinAngle = _lerp(
		hi->m_measures.m_inertion_params.m_min_angle,
		hi->m_measures.m_inertion_params.m_min_angle_aim,
		m_zoom_params.m_fZoomRotationFactor);

	Fvector4 vIOffsets; // x = L, y = R, z = U, w = D
	vIOffsets.x = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.x,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.x,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.y = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.y,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.y,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.z = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.z,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.z,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.w = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.w,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.w,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;

	// ����������� ������� �� ��������� ������
	bool bIsInertionPresent = m_fLR_InertiaFactor != 0.0f || m_fUD_InertiaFactor != 0.0f;
	if (abs(fYMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fYMag > 0.0f && m_fLR_InertiaFactor > 0.0f ||
			fYMag < 0.0f && m_fLR_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> �������� ������� ��� �������� � ��������������� �������
		}

		m_fLR_InertiaFactor -= (fYMag * fAvgTimeDelta * fSpeed); // ����������� (�.�. > |1.0|)
	}

	if (abs(fPMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fPMag > 0.0f && m_fUD_InertiaFactor > 0.0f ||
			fPMag < 0.0f && m_fUD_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> �������� ������� ��� �������� � ��������������� �������
		}

		m_fUD_InertiaFactor -= (fPMag * fAvgTimeDelta * fSpeed); // ��������� (�.�. > |1.0|)
	}

	clamp(m_fLR_InertiaFactor, -1.0f, 1.0f);
	clamp(m_fUD_InertiaFactor, -1.0f, 1.0f);

	// ������� ��������� ������� (��������, �� ��� �������� ������� �� ������� ������� �� ������� 0.0f)
	m_fLR_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);
	m_fUD_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);

	// ����������� �������� ��������� ������� ��� ����� (�����������)
	if (fYMag == 0.0f)
	{
		float fRetSpeedMod = (fYMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fLR_InertiaFactor < 0.0f)
		{
			m_fLR_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// ����������� �������� ��������� ������� ��� ����� (���������)
	if (fPMag == 0.0f)
	{
		float fRetSpeedMod = (fPMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fUD_InertiaFactor < 0.0f)
		{
			m_fUD_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fUD_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// ��������� ������� � ����
	float fLR_lim = (m_fLR_InertiaFactor < 0.0f ? vIOffsets.x : vIOffsets.y);
	float fUD_lim = (m_fUD_InertiaFactor < 0.0f ? vIOffsets.z : vIOffsets.w);

	Fvector curr_offs;
	curr_offs = { fLR_lim * -1.f * m_fLR_InertiaFactor, fUD_lim * m_fUD_InertiaFactor, 0.0f };

	Fmatrix hud_rotation;
	hud_rotation.identity();
	hud_rotation.translate_over(curr_offs);
	trans.mulB_43(hud_rotation);
}