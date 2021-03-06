function [seq,cls,ref]=modifySequence(seq0,cls0,removedCLS,delay);
%remove the item of class *removedCLS*. Add delay to the seq, and return sorted result

n=length(delay);
%remove
picked=find(cls0!=removedCLS);
seq=seq0(picked);  cls=cls0(picked);
if(nargout==3)
  ref=seq0(find(cls0==removedCLS));
end
%delay
for i=1:n
  if(i==removedCLS)
    continue;
  end
  picked=find(cls==i);
  seq(picked)+=delay(i);
end
%re-sort
[seq,t]=sort(seq);
cls=cls(t);

end
