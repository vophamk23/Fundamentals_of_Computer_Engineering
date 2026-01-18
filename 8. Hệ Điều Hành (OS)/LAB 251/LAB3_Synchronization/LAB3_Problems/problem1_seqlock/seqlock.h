#ifndef SEQLOCK_H
#define SEQLOCK_H

#ifndef PTHREAD_H
#include <pthread.h>
#endif

// Cấu trúc Sequence Lock
typedef struct pthread_seqlock
{
   unsigned int sequence; // Số thứ tự (chẵn/lẻ để báo hiệu trạng thái)
   pthread_mutex_t lock;  // Mutex bảo vệ writer (chỉ 1 writer tại 1 thời điểm)
   unsigned int read_seq; // Lưu sequence khi bắt đầu đọc (cho reader)
} pthread_seqlock_t;

/**
 * Khởi tạo sequence lock
 * @param rw: Con trỏ tới seqlock
 */
static inline void pthread_seqlock_init(pthread_seqlock_t *rw)
{
   rw->sequence = 0; // Bắt đầu với số CHẴN (không có writer)
   rw->read_seq = 0;
   pthread_mutex_init(&rw->lock, NULL);
}

/**
 * Writer lock: Bắt đầu ghi
 * - Chỉ 1 writer được phép vào critical section
 * - Tăng sequence lên số LẺ để báo hiệu đang ghi
 */
static inline void pthread_seqlock_wrlock(pthread_seqlock_t *rw)
{
   // Lock mutex - đảm bảo chỉ 1 writer
   pthread_mutex_lock(&rw->lock);

   // Tăng sequence lên số LẺ (chẵn + 1 = lẻ)
   // Atomic operation để đảm bảo thread-safe
   __atomic_fetch_add(&rw->sequence, 1, __ATOMIC_SEQ_CST);
}

/**
 * Writer unlock: Kết thúc ghi
 * - Tăng sequence lên số CHẴN để báo hiệu ghi xong
 */
static inline void pthread_seqlock_wrunlock(pthread_seqlock_t *rw)
{
   // Tăng sequence lên số CHẴN (lẻ + 1 = chẵn)
   __atomic_fetch_add(&rw->sequence, 1, __ATOMIC_SEQ_CST);

   // Unlock mutex
   pthread_mutex_unlock(&rw->lock);
}

/**
 * Reader lock: Bắt đầu đọc
 * @return 1 nếu có thể đọc (sequence chẵn), 0 nếu không (sequence lẻ - writer đang ghi)
 *
 * Cơ chế:
 * - Đọc sequence hiện tại
 * - Nếu CHẴN → không có writer → có thể đọc
 * - Nếu LẺ → writer đang ghi → không thể đọc
 */
static inline unsigned pthread_seqlock_rdlock(pthread_seqlock_t *rw)
{
   unsigned int seq;

   // Đọc sequence number hiện tại (atomic)
   seq = __atomic_load_n(&rw->sequence, __ATOMIC_SEQ_CST);

   // Lưu lại sequence để kiểm tra sau khi đọc xong
   rw->read_seq = seq;

   // Kiểm tra bit cuối:
   // - Nếu seq & 1 == 0 → CHẴN → return 1 (có thể đọc)
   // - Nếu seq & 1 == 1 → LẺ → return 0 (không thể đọc)
   if (seq & 1)
   {
      return 0; // Sequence LẺ - writer đang ghi
   }

   return 1; // Sequence CHẴN - có thể đọc
}

/**
 * Reader unlock: Kết thúc đọc và kiểm tra tính nhất quán
 * @return 1 nếu dữ liệu hợp lệ (không có writer chen ngang)
 *         0 nếu dữ liệu không hợp lệ (có writer chen ngang - cần retry)
 *
 * Cơ chế:
 * - Đọc lại sequence
 * - So sánh với sequence lúc bắt đầu đọc
 * - Nếu GIỐNG NHAU → không có writer chen ngang → dữ liệu hợp lệ
 * - Nếu KHÁC NHAU → có writer đã ghi → dữ liệu không nhất quán → cần retry
 */
static inline unsigned pthread_seqlock_rdunlock(pthread_seqlock_t *rw)
{
   // Đọc sequence hiện tại
   unsigned int seq = __atomic_load_n(&rw->sequence, __ATOMIC_SEQ_CST);

   // So sánh với sequence lúc bắt đầu đọc
   if (rw->read_seq != seq)
   {
      return 0; // Sequence thay đổi - có writer chen ngang - dữ liệu không hợp lệ
   }

   return 1; // Sequence không đổi - dữ liệu hợp lệ
}

#endif // SEQLOCK_H