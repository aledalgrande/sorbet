
VALUE sorbet_rb_int_plus_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_plus_fix(recv, y);
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_plus(y, recv);
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return DBL2NUM((double)FIX2LONG(recv) + RFLOAT_VALUE(y));
        } else if (RB_TYPE_P(y, T_COMPLEX)) {
            return rb_complex_plus(y, recv);
        }
        // fall through to coerce
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_plus(recv, y);
    }
    return rb_num_coerce_bin(recv, y, '+');
}

VALUE sorbet_rb_int_minus_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            return rb_fix_minus_fix(recv, y);
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            VALUE x = rb_int2big(FIX2LONG(recv));
            return rb_big_minus(x, y);
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return DBL2NUM((double)FIX2LONG(recv) - RFLOAT_VALUE(y));
        }
        // fall throught to coerece
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_minus(recv, y);
    }
    return rb_num_coerce_bin(recv, y, '-');
}

VALUE sorbet_rb_int_gt_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) > FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(-1) ? Qtrue : Qfalse;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(+1) ? Qtrue : Qfalse;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_gt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '>');
}

VALUE sorbet_rb_int_lt_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) < FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(+1) ? Qtrue : Qfalse;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(-1) ? Qtrue : Qfalse;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_lt(recv, y);
    }
    return rb_num_coerce_relop(recv, y, '<');
}

VALUE sorbet_rb_int_ge_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) >= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(+1) ? Qfalse : Qtrue;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(-1) ? Qfalse : Qtrue;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_ge(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idGE);
}

VALUE sorbet_rb_int_le_slowpath(VALUE recv, VALUE y) {
    if (LIKELY(FIXNUM_P(recv))) {
        if (LIKELY(FIXNUM_P(y))) {
            if (FIX2LONG(recv) <= FIX2LONG(y)) {
                return Qtrue;
            }
            return Qfalse;
        } else if (RB_TYPE_P(y, T_BIGNUM)) {
            return rb_big_cmp(y, recv) == INT2FIX(-1) ? Qfalse : Qtrue;
        } else if (RB_TYPE_P(y, T_FLOAT)) {
            return rb_integer_float_cmp(recv, y) == INT2FIX(+1) ? Qfalse : Qtrue;
        }
    } else if (RB_TYPE_P(recv, T_BIGNUM)) {
        return rb_big_le(recv, y);
    }
    return rb_num_coerce_relop(recv, y, idLE);
}
