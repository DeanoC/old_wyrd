#pragma once

#include <cmath>
#include <memory>
#include <cfloat>
#include "imgui/imgui.h"

namespace ImGuizmoMath {
    static const float ZPI = 3.14159265358979323846f;
    static const float RAD2DEG = (180.f / ZPI);
    static const float DEG2RAD = (ZPI / 180.f);

    const float screenRotateSize = 0.06f;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // utility and math

    void FPU_MatrixF_x_MatrixF(const float *a, const float *b, float *r) {
        r[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
        r[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
        r[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
        r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

        r[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
        r[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
        r[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
        r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

        r[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
        r[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
        r[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
        r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

        r[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
        r[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
        r[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
        r[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
    }

    //template <typename T> T LERP(T x, T y, float z) { return (x + (y - x)*z); }
    template<typename T>
    T Clamp(T x, T y, T z) { return ((x < y) ? y : ((x > z) ? z : x)); }

    template<typename T>
    T max(T x, T y) { return (x > y) ? x : y; }

    template<typename T>
    T min(T x, T y) { return (x < y) ? x : y; }

    template<typename T>
    bool IsWithin(T x, T y, T z) { return (x >= y) && (x <= z); }

    struct matrix_t;

    struct vec_t {
    public:
        float x, y, z, w;

        void Lerp(const vec_t &v, float t) {
            x += (v.x - x) * t;
            y += (v.y - y) * t;
            z += (v.z - z) * t;
            w += (v.w - w) * t;
        }

        void Set(float v) { x = y = z = w = v; }

        void Set(float _x, float _y, float _z = 0.f, float _w = 0.f) {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
        }

        vec_t &operator-=(const vec_t &v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            return *this;
        }

        vec_t &operator+=(const vec_t &v) {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            return *this;
        }

        vec_t &operator*=(const vec_t &v) {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            w *= v.w;
            return *this;
        }

        vec_t &operator*=(float v) {
            x *= v;
            y *= v;
            z *= v;
            w *= v;
            return *this;
        }

        vec_t operator*(float f) const;

        vec_t operator-() const;

        vec_t operator-(const vec_t &v) const;

        vec_t operator+(const vec_t &v) const;

        vec_t operator*(const vec_t &v) const;

        const vec_t &operator+() const { return (*this); }

        float Length() const { return sqrtf(x * x + y * y + z * z); };

        float LengthSq() const { return (x * x + y * y + z * z); };

        vec_t Normalize() {
            (*this) *= (1.f / Length());
            return (*this);
        }

        vec_t Normalize(const vec_t &v) {
            this->Set(v.x, v.y, v.z, v.w);
            this->Normalize();
            return (*this);
        }

        vec_t Abs() const;

        void Cross(const vec_t &v) {
            vec_t res;
            res.x = y * v.z - z * v.y;
            res.y = z * v.x - x * v.z;
            res.z = x * v.y - y * v.x;

            x = res.x;
            y = res.y;
            z = res.z;
            w = 0.f;
        }

        void Cross(const vec_t &v1, const vec_t &v2) {
            x = v1.y * v2.z - v1.z * v2.y;
            y = v1.z * v2.x - v1.x * v2.z;
            z = v1.x * v2.y - v1.y * v2.x;
            w = 0.f;
        }

        float Dot(const vec_t &v) const {
            return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
        }

        float Dot3(const vec_t &v) const {
            return (x * v.x) + (y * v.y) + (z * v.z);
        }

        void Transform(const matrix_t &matrix);

        void Transform(const vec_t &s, const matrix_t &matrix);

        void TransformVector(const matrix_t &matrix);

        void TransformPoint(const matrix_t &matrix);

        void TransformVector(const vec_t &v, const matrix_t &matrix) {
            (*this) = v;
            this->TransformVector(matrix);
        }

        void TransformPoint(const vec_t &v, const matrix_t &matrix) {
            (*this) = v;
            this->TransformPoint(matrix);
        }

        float &operator[](size_t index) { return ((float *) &x)[index]; }

        const float &operator[](size_t index) const { return ((float *) &x)[index]; }
    };

    vec_t makeVect(float _x, float _y, float _z = 0.f, float _w = 0.f) {
        vec_t res;
        res.x = _x;
        res.y = _y;
        res.z = _z;
        res.w = _w;
        return res;
    }

    vec_t vec_t::operator*(float f) const { return makeVect(x * f, y * f, z * f, w * f); }

    vec_t vec_t::operator-() const { return makeVect(-x, -y, -z, -w); }

    vec_t vec_t::operator-(const vec_t &v) const { return makeVect(x - v.x, y - v.y, z - v.z, w - v.w); }

    vec_t vec_t::operator+(const vec_t &v) const { return makeVect(x + v.x, y + v.y, z + v.z, w + v.w); }

    vec_t vec_t::operator*(const vec_t &v) const { return makeVect(x * v.x, y * v.y, z * v.z, w * v.w); }

    vec_t vec_t::Abs() const { return makeVect(fabsf(x), fabsf(y), fabsf(z)); }

    vec_t Normalized(const vec_t &v) {
        vec_t res;
        res = v;
        res.Normalize();
        return res;
    }

    vec_t Cross(const vec_t &v1, const vec_t &v2) {
        vec_t res;
        res.x = v1.y * v2.z - v1.z * v2.y;
        res.y = v1.z * v2.x - v1.x * v2.z;
        res.z = v1.x * v2.y - v1.y * v2.x;
        res.w = 0.f;
        return res;
    }

    float Dot(const vec_t &v1, const vec_t &v2) {
        return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
    }

    vec_t BuildPlan(const vec_t &p_point1, const vec_t &p_normal) {
        vec_t normal, res;
        normal.Normalize(p_normal);
        res.w = normal.Dot(p_point1);
        res.x = normal.x;
        res.y = normal.y;
        res.z = normal.z;
        return res;
    }

    struct matrix_t {
    public:

        union {
            float m[4][4];
            float m16[16];
            struct {
                vec_t right, up, dir, position;
            } v;
            vec_t component[4];
        };

        matrix_t(const matrix_t &other) { memcpy(&m16[0], &other.m16[0], sizeof(float) * 16); }

        matrix_t() {}

        operator float *() { return m16; }

        operator const float *() const { return m16; }

        void Translation(float _x, float _y, float _z) { this->Translation(makeVect(_x, _y, _z)); }

        void Translation(const vec_t &vt) {
            v.right.Set(1.f, 0.f, 0.f, 0.f);
            v.up.Set(0.f, 1.f, 0.f, 0.f);
            v.dir.Set(0.f, 0.f, 1.f, 0.f);
            v.position.Set(vt.x, vt.y, vt.z, 1.f);
        }

        void Scale(float _x, float _y, float _z) {
            v.right.Set(_x, 0.f, 0.f, 0.f);
            v.up.Set(0.f, _y, 0.f, 0.f);
            v.dir.Set(0.f, 0.f, _z, 0.f);
            v.position.Set(0.f, 0.f, 0.f, 1.f);
        }

        void Scale(const vec_t &s) { Scale(s.x, s.y, s.z); }

        matrix_t &operator*=(const matrix_t &mat) {
            matrix_t tmpMat;
            tmpMat = *this;
            tmpMat.Multiply(mat);
            *this = tmpMat;
            return *this;
        }

        matrix_t operator*(const matrix_t &mat) const {
            matrix_t matT;
            matT.Multiply(*this, mat);
            return matT;
        }

        void Multiply(const matrix_t &matrix) {
            matrix_t tmp;
            tmp = *this;

            FPU_MatrixF_x_MatrixF((float *) &tmp, (float *) &matrix, (float *) this);
        }

        void Multiply(const matrix_t &m1, const matrix_t &m2) {
            FPU_MatrixF_x_MatrixF((float *) &m1, (float *) &m2, (float *) this);
        }

        float GetDeterminant() const {
            return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1] -
                   m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
        }

        float Inverse(const matrix_t &srcMatrix, bool affine = false);

        void SetToIdentity() {
            v.right.Set(1.f, 0.f, 0.f, 0.f);
            v.up.Set(0.f, 1.f, 0.f, 0.f);
            v.dir.Set(0.f, 0.f, 1.f, 0.f);
            v.position.Set(0.f, 0.f, 0.f, 1.f);
        }

        void Transpose() {
            matrix_t tmpm;
            for (int l = 0; l < 4; l++) {
                for (int c = 0; c < 4; c++) {
                    tmpm.m[l][c] = m[c][l];
                }
            }
            (*this) = tmpm;
        }

        void RotationAxis(const vec_t &axis, float angle);

        void OrthoNormalize() {
            v.right.Normalize();
            v.up.Normalize();
            v.dir.Normalize();
        }
    };

    void vec_t::Transform(const matrix_t &matrix) {
        vec_t out;

        out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] + w * matrix.m[3][0];
        out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] + w * matrix.m[3][1];
        out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] + w * matrix.m[3][2];
        out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3] + w * matrix.m[3][3];

        x = out.x;
        y = out.y;
        z = out.z;
        w = out.w;
    }

    void vec_t::Transform(const vec_t &s, const matrix_t &matrix) {
        *this = s;
        Transform(matrix);
    }

    void vec_t::TransformPoint(const matrix_t &matrix) {
        vec_t out;

        out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] + matrix.m[3][0];
        out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] + matrix.m[3][1];
        out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] + matrix.m[3][2];
        out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3] + matrix.m[3][3];

        x = out.x;
        y = out.y;
        z = out.z;
        w = out.w;
    }


    void vec_t::TransformVector(const matrix_t &matrix) {
        vec_t out;

        out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0];
        out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1];
        out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2];
        out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3];

        x = out.x;
        y = out.y;
        z = out.z;
        w = out.w;
    }

    float matrix_t::Inverse(const matrix_t &srcMatrix, bool affine) {
        float det = 0;

        if (affine) {
            det = GetDeterminant();
            float s = 1 / det;
            m[0][0] = (srcMatrix.m[1][1] * srcMatrix.m[2][2] - srcMatrix.m[1][2] * srcMatrix.m[2][1]) * s;
            m[0][1] = (srcMatrix.m[2][1] * srcMatrix.m[0][2] - srcMatrix.m[2][2] * srcMatrix.m[0][1]) * s;
            m[0][2] = (srcMatrix.m[0][1] * srcMatrix.m[1][2] - srcMatrix.m[0][2] * srcMatrix.m[1][1]) * s;
            m[1][0] = (srcMatrix.m[1][2] * srcMatrix.m[2][0] - srcMatrix.m[1][0] * srcMatrix.m[2][2]) * s;
            m[1][1] = (srcMatrix.m[2][2] * srcMatrix.m[0][0] - srcMatrix.m[2][0] * srcMatrix.m[0][2]) * s;
            m[1][2] = (srcMatrix.m[0][2] * srcMatrix.m[1][0] - srcMatrix.m[0][0] * srcMatrix.m[1][2]) * s;
            m[2][0] = (srcMatrix.m[1][0] * srcMatrix.m[2][1] - srcMatrix.m[1][1] * srcMatrix.m[2][0]) * s;
            m[2][1] = (srcMatrix.m[2][0] * srcMatrix.m[0][1] - srcMatrix.m[2][1] * srcMatrix.m[0][0]) * s;
            m[2][2] = (srcMatrix.m[0][0] * srcMatrix.m[1][1] - srcMatrix.m[0][1] * srcMatrix.m[1][0]) * s;
            m[3][0] = -(m[0][0] * srcMatrix.m[3][0] + m[1][0] * srcMatrix.m[3][1] + m[2][0] * srcMatrix.m[3][2]);
            m[3][1] = -(m[0][1] * srcMatrix.m[3][0] + m[1][1] * srcMatrix.m[3][1] + m[2][1] * srcMatrix.m[3][2]);
            m[3][2] = -(m[0][2] * srcMatrix.m[3][0] + m[1][2] * srcMatrix.m[3][1] + m[2][2] * srcMatrix.m[3][2]);
        } else {
            // transpose matrix
            float src[16];
            for (int i = 0; i < 4; ++i) {
                src[i] = srcMatrix.m16[i * 4];
                src[i + 4] = srcMatrix.m16[i * 4 + 1];
                src[i + 8] = srcMatrix.m16[i * 4 + 2];
                src[i + 12] = srcMatrix.m16[i * 4 + 3];
            }

            // calculate pairs for first 8 elements (cofactors)
            float tmp[12]; // temp array for pairs
            tmp[0] = src[10] * src[15];
            tmp[1] = src[11] * src[14];
            tmp[2] = src[9] * src[15];
            tmp[3] = src[11] * src[13];
            tmp[4] = src[9] * src[14];
            tmp[5] = src[10] * src[13];
            tmp[6] = src[8] * src[15];
            tmp[7] = src[11] * src[12];
            tmp[8] = src[8] * src[14];
            tmp[9] = src[10] * src[12];
            tmp[10] = src[8] * src[13];
            tmp[11] = src[9] * src[12];

            // calculate first 8 elements (cofactors)
            m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7]) -
                     (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
            m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7]) -
                     (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
            m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) -
                     (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
            m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) -
                     (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
            m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3]) -
                     (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
            m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3]) -
                     (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
            m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) -
                     (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
            m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) -
                     (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

            // calculate pairs for second 8 elements (cofactors)
            tmp[0] = src[2] * src[7];
            tmp[1] = src[3] * src[6];
            tmp[2] = src[1] * src[7];
            tmp[3] = src[3] * src[5];
            tmp[4] = src[1] * src[6];
            tmp[5] = src[2] * src[5];
            tmp[6] = src[0] * src[7];
            tmp[7] = src[3] * src[4];
            tmp[8] = src[0] * src[6];
            tmp[9] = src[2] * src[4];
            tmp[10] = src[0] * src[5];
            tmp[11] = src[1] * src[4];

            // calculate second 8 elements (cofactors)
            m16[8] = (tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15]) -
                     (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
            m16[9] = (tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15]) -
                     (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
            m16[10] = (tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15]) -
                      (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
            m16[11] = (tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14]) -
                      (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
            m16[12] = (tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9]) -
                      (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
            m16[13] = (tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10]) -
                      (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
            m16[14] = (tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8]) -
                      (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
            m16[15] = (tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9]) -
                      (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);

            // calculate determinant
            det = src[0] * m16[0] + src[1] * m16[1] + src[2] * m16[2] + src[3] * m16[3];

            // calculate matrix inverse
            float invdet = 1 / det;
            for (int j = 0; j < 16; ++j) {
                m16[j] *= invdet;
            }
        }

        return det;
    }

    void matrix_t::RotationAxis(const vec_t &axis, float angle) {
        float length2 = axis.LengthSq();
        if (length2 < FLT_EPSILON) {
            SetToIdentity();
            return;
        }

        vec_t n = axis * (1.f / sqrtf(length2));
        float s = sinf(angle);
        float c = cosf(angle);
        float k = 1.f - c;

        float xx = n.x * n.x * k + c;
        float yy = n.y * n.y * k + c;
        float zz = n.z * n.z * k + c;
        float xy = n.x * n.y * k;
        float yz = n.y * n.z * k;
        float zx = n.z * n.x * k;
        float xs = n.x * s;
        float ys = n.y * s;
        float zs = n.z * s;

        m[0][0] = xx;
        m[0][1] = xy + zs;
        m[0][2] = zx - ys;
        m[0][3] = 0.f;
        m[1][0] = xy - zs;
        m[1][1] = yy;
        m[1][2] = yz + xs;
        m[1][3] = 0.f;
        m[2][0] = zx + ys;
        m[2][1] = yz - xs;
        m[2][2] = zz;
        m[2][3] = 0.f;
        m[3][0] = 0.f;
        m[3][1] = 0.f;
        m[3][2] = 0.f;
        m[3][3] = 1.f;
    }

    struct Viewport {
        float width;
        float height;
        float x;
        float y;
    };

    inline ImVec2 worldToPos(const vec_t& worldPos, const matrix_t& mat, Viewport const& viewport)
    {
        vec_t trans;
        trans.TransformPoint(worldPos, mat);
        trans *= 0.5f / trans.w;
        trans += makeVect(0.5f, 0.5f);
        trans.y = 1.f - trans.y;
        trans.x *= viewport.width;
        trans.y *= viewport.height;
        trans.x += viewport.x;
        trans.y += viewport.y;
        return ImVec2(trans.x, trans.y);
    }

    inline void ComputeCameraRay(vec_t &rayOrigin, vec_t &rayDir, matrix_t const& viewProjInverse, Viewport const& viewport)
    {
        ImGuiIO& io = ImGui::GetIO();

        float mox = ((io.MousePos.x - viewport.x) / viewport.width) * 2.f - 1.f;
        float moy = (1.f - ((io.MousePos.y - viewport.y) / viewport.height)) * 2.f - 1.f;

        rayOrigin.Transform(makeVect(mox, moy, 0.f, 1.f), viewProjInverse);
        rayOrigin *= 1.f / rayOrigin.w;
        vec_t rayEnd;
        rayEnd.Transform(makeVect(mox, moy, 1.f, 1.f), viewProjInverse);
        rayEnd *= 1.f / rayEnd.w;
        rayDir = Normalized(rayEnd - rayOrigin);
    }

    inline float IntersectRayPlane(const vec_t & rOrigin, const vec_t& rVector, const vec_t& plan)
    {
        float numer = plan.Dot3(rOrigin) - plan.w;
        float denom = plan.Dot3(rVector);

        if (fabsf(denom) < FLT_EPSILON)  // normal is orthogonal to vector, cant intersect
            return -1.0f;

        return -(numer / denom);
    }
}
