#include "GeometricPrimitive.h"

GeometricPrimitive::GeometricPrimitive(ID3D11Device* device)
{
    Vertex  vertices[24]{};
    //正立方体のコントロールポイント数は8個、
    //法線の向きが違う頂点が３個あるので頂点情報の総数は8x3 = 24個、
    //頂点情報配列（vertices）にすべて頂点の位置・法線情報を格納する。

    uint32_t indices[36]{};
    //正立方体は６面持ち、１つの面は２つの３角形ポリゴンの総数は6x2 = 12個、
    //正立方体を描画するために１２回の３角形ポリゴン描画が必要、よって参照される頂点情報は12x3 = 36回、
    //3角形ポリゴンが参照する頂点情報のインデックス（頂点番号）を描画順に配列（indices）に格納する。
    //時計回りは￥が表面になるように格納すること。
}
